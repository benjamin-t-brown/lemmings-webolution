addToLibrary({
  $WebolutionAudio: {
    mixers: null,

    // A source owns its clip on the audio rendering timeline. Once started,
    // it can play and loop during a long main-thread render without refills.
    voice(mixer, clip, bus, volume, left, right, loop, now) {
      const {context} = mixer;
      const source = context.createBufferSource();
      source.buffer = clip;
      source.loop = loop;

      const gain = context.createGain();
      source.connect(gain);
      let output = gain;
      if (mixer.channels === 1 || left === right) {
        gain.gain.value = volume * (left + right) / 2;
      } else {
        // Apply per-channel gain, duplicating mono only at playback.
        const split = context.createChannelSplitter(clip.numberOfChannels);
        const merge = context.createChannelMerger(2);
        gain.gain.value = volume;
        gain.connect(split);
        for (const [channel, level] of [left, right].entries()) {
          const side = context.createGain();
          side.gain.value = level;
          split.connect(side, clip.numberOfChannels === 1 ? 0 : channel);
          side.connect(merge, 0, channel);
        }
        output = merge;
      }
      output.connect(bus);
      const voice = {source, output, end: loop ? Infinity : now + clip.duration, average: false};
      source.onended = () => {
        mixer.effects.delete(voice);
        source.disconnect();
        output.disconnect();
      };
      source.start(now);
      return voice;
    },

    stop(mixer, voice) {
      voice.source.stop();
      voice.output.disconnect();
      mixer.effects.delete(voice);
      if (mixer.music === voice) mixer.music = null;
    },

    // Exit cheers average over the voices still active at each sample. Queue
    // the gain changes at the tails now so they also work during a long draw.
    updateExitGain(mixer, now) {
      const ends = [];
      for (const voice of mixer.effects) {
        if (voice.average && voice.end > now) ends.push(voice.end);
      }
      ends.sort((a, b) => a - b);
      const gain = mixer.exitBus.gain;
      gain.cancelScheduledValues(now);
      gain.setValueAtTime(1 / Math.max(1, ends.length), now);
      for (const [index, end] of ends.entries()) {
        if (end !== ends[index + 1]) {
          gain.setValueAtTime(1 / Math.max(1, ends.length - index - 1), end);
        }
      }
    },
  },

  weboAudioOpen__deps: ['$WebolutionAudio'],
  weboAudioOpen(id, channels, maxVoices, musicVolume, sfxVolume) {
    const AudioContext = globalThis.AudioContext || globalThis.webkitAudioContext;
    if (!AudioContext) return 0;
    let context;
    try {
      context = new AudioContext({latencyHint: 'interactive'});
      const musicBus = context.createGain();
      const effectsBus = context.createGain();
      const exitBus = context.createGain();
      musicBus.connect(context.destination);
      effectsBus.connect(context.destination);
      exitBus.connect(effectsBus);
      musicBus.gain.value = musicVolume;
      effectsBus.gain.value = sfxVolume;
      // Start calls into Wasm directly from the page's click handler.
      if (context.state === 'suspended') context.resume().catch(() => {});
      WebolutionAudio.mixers ??= new Map();
      WebolutionAudio.mixers.set(id, {context, musicBus, effectsBus, exitBus, channels, maxVoices,
        clips: new Map(), effects: new Set(), music: null});
      return 1;
    } catch (error) {
      if (context) context.close().catch(() => {});
      return 0;
    }
  },

  weboAudioClose__deps: ['$WebolutionAudio'],
  weboAudioClose(id) {
    const mixer = WebolutionAudio.mixers.get(id);
    WebolutionAudio.mixers.delete(id);
    mixer.context.close().catch(() => {});
  },

  weboAudioLoad__deps: ['$WebolutionAudio', '$UTF8ToString'],
  weboAudioLoad(id, key, pcm, frames, channels, sampleRate) {
    const mixer = WebolutionAudio.mixers.get(id);
    const name = UTF8ToString(key);
    let clip;
    try {
      clip = mixer.context.createBuffer(channels, frames, sampleRate);
      const start = pcm >>> 1;
      const samples = HEAP16.subarray(start, start + frames * channels);
      for (let channel = 0; channel < channels; ++channel) {
        const output = clip.getChannelData(channel);
        for (let frame = 0; frame < frames; ++frame) {
          output[frame] = samples[frame * channels + channel] / 32768;
        }
      }
    } catch (error) {
      return 0;
    }
    const old = mixer.clips.get(name);
    if (old) {
      if (mixer.music?.source.buffer === old) WebolutionAudio.stop(mixer, mixer.music);
      let averaged = false;
      for (const voice of mixer.effects) {
        if (voice.source.buffer !== old) continue;
        averaged ||= voice.average;
        WebolutionAudio.stop(mixer, voice);
      }
      if (averaged) WebolutionAudio.updateExitGain(mixer, mixer.context.currentTime);
    }
    mixer.clips.set(name, clip);
    return 1;
  },

  weboAudioHas__deps: ['$WebolutionAudio', '$UTF8ToString'],
  weboAudioHas(id, key) {
    return WebolutionAudio.mixers.get(id).clips.has(UTF8ToString(key)) ? 1 : 0;
  },

  weboAudioPlay__deps: ['$WebolutionAudio', '$UTF8ToString'],
  weboAudioPlay(id, key, volume, left, right, average, skipPlaying, restartPlaying, addWithoutEviction) {
    const mixer = WebolutionAudio.mixers.get(id);
    const clip = mixer.clips.get(UTF8ToString(key));
    if (!clip) return;
    const now = mixer.context.currentTime;
    if (mixer.music?.source.buffer === clip) {
      if (skipPlaying) return;
      if (restartPlaying) WebolutionAudio.stop(mixer, mixer.music);
    }
    let oldest;
    let additive = 0;
    let exitChanged = false;
    for (const voice of mixer.effects) {
      if (voice.end <= now) continue; // Main-thread ended notification can be late.
      if (voice.source.buffer === clip) {
        if (skipPlaying) return;
        if (restartPlaying) {
          exitChanged ||= voice.average;
          WebolutionAudio.stop(mixer, voice);
          continue;
        }
      }
      if (!voice.average && !voice.addWithoutEviction) {
        oldest ??= voice;
        ++additive;
      }
    }
    if (!average && !addWithoutEviction && additive >= mixer.maxVoices)
      WebolutionAudio.stop(mixer, oldest);
    const bus = average ? mixer.exitBus : mixer.effectsBus;
    const voice = WebolutionAudio.voice(mixer, clip, bus, volume, left, right, false, now);
    voice.average = Boolean(average);
    voice.addWithoutEviction = Boolean(addWithoutEviction);
    mixer.effects.add(voice);
    if (average || exitChanged) WebolutionAudio.updateExitGain(mixer, now);
  },

  weboAudioPlayMusic__deps: ['$WebolutionAudio', '$UTF8ToString'],
  weboAudioPlayMusic(id, key, volume) {
    const mixer = WebolutionAudio.mixers.get(id);
    if (mixer.music) WebolutionAudio.stop(mixer, mixer.music);
    const clip = mixer.clips.get(UTF8ToString(key));
    if (clip) {
      mixer.music = WebolutionAudio.voice(mixer, clip, mixer.musicBus, volume, 1, 1,
        true, mixer.context.currentTime);
    }
  },

  weboAudioStopMusic__deps: ['$WebolutionAudio'],
  weboAudioStopMusic(id) {
    const mixer = WebolutionAudio.mixers.get(id);
    if (mixer.music) WebolutionAudio.stop(mixer, mixer.music);
  },

  weboAudioStopAll__deps: ['$WebolutionAudio'],
  weboAudioStopAll(id) {
    const mixer = WebolutionAudio.mixers.get(id);
    if (mixer.music) WebolutionAudio.stop(mixer, mixer.music);
    for (const voice of mixer.effects) WebolutionAudio.stop(mixer, voice);
    WebolutionAudio.updateExitGain(mixer, mixer.context.currentTime);
  },

  weboAudioVolumes__deps: ['$WebolutionAudio'],
  weboAudioVolumes(id, music, sfx) {
    const mixer = WebolutionAudio.mixers.get(id);
    mixer.musicBus.gain.value = music;
    mixer.effectsBus.gain.value = sfx;
  },
});

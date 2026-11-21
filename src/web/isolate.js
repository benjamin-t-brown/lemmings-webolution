// Supply isolation headers on hosts that serve static files as-is.
self.addEventListener('install', event => event.waitUntil(self.skipWaiting()));
self.addEventListener('activate', event => event.waitUntil(self.clients.claim()));

self.addEventListener('fetch', event => {
  const {request} = event;
  // External CORS requests use the browser's normal fetch path.
  if (!request.url.startsWith(self.registration.scope)) return;
  event.respondWith((async () => {
    const response = await fetch(request);
    if (response.type === 'opaque' || response.type === 'opaqueredirect') return response;
    const headers = new Headers(response.headers);
    headers.set('Cross-Origin-Opener-Policy', 'same-origin');
    headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
    return new Response(response.body, {
      status: response.status,
      statusText: response.statusText,
      headers,
    });
  })());
});

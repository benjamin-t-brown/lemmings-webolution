"""Serve the Wasm game and the two installed archives on loopback only."""

from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit


ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build/cmake/emscripten-release"
GAME = ROOT / "Lemmings Revolution"
FILES = {
    "/": BUILD / "webolution.html",
    **{f"/webolution.{ext}": BUILD / f"webolution.{ext}" for ext in ("html", "js", "wasm")},
    "/web-game-data.mjs": BUILD / "web-game-data.mjs",
    "/isolate.js": BUILD / "isolate.js",
    "/favicon.png": BUILD / "favicon.png",
    **{f"/game/{name}": GAME / name for name in ("lemmings.box", "patch.box")},
}


class GameHandler(SimpleHTTPRequestHandler):
    extensions_map = {".wasm": "application/wasm", ".js": "text/javascript", ".mjs": "text/javascript",
                      ".png": "image/png"}

    def send_head(self):
        if urlsplit(self.path).path not in FILES:
            self.send_error(HTTPStatus.NOT_FOUND)
            return None
        return super().send_head()

    def translate_path(self, path):
        return str(FILES[urlsplit(path).path])

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()


if __name__ == "__main__":
    for file in set(FILES.values()):
        if not file.is_file():
            raise SystemExit(f"Missing {file}")
    with ThreadingHTTPServer(("127.0.0.1", 8000), GameHandler) as server:
        print("Open http://127.0.0.1:8000/  (Ctrl+C to stop)", flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass

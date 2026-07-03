from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


class QuietHandler(SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        return


if __name__ == "__main__":
    base_dir = Path(__file__).resolve().parent
    handler = lambda *args, **kwargs: QuietHandler(*args, directory=str(base_dir), **kwargs)
    server = ThreadingHTTPServer(("127.0.0.1", 8765), handler)
    server.serve_forever()

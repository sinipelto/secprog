import http.server
import ssl

httpd = http.server.HTTPServer(('0.0.0.0', 443), http.server.SimpleHTTPRequestHandler)
httpd.socket = ssl.wrap_socket (httpd.socket, certfile='./server.pem', keyfile='./key.pem', server_side=True)
httpd.serve_forever()

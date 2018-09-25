# Asynchronous http(s) client

Example of use:

- HTTP
```sh
$ ./main http://example.com:80 /index.html
```
You can also pass a parameter without a protocol or(and) port (by default, the HTTP is used, port 80).


- HTTPS
```sh
$ ./main https://www.boost.org:443 /LICENSE_1_0.txt
```
The port can be omitted (default port is 443). The protocol must be specified.


Example of SSL certificate verify failed:

```sh
$ ./main https://self-signed.badssl.com /index.html
Connect OK 
Verifying /C=US/ST=California/L=San Francisco/O=BadSSL Fallback. Unknown subdomain or no SNI./CN=badssl-fallback-unknown-subdomain-or-no-sni
Handshake failed: certificate verify failed
```

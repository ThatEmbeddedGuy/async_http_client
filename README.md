# Asynchronous http(s) client

Example of use:

- HTTP
```sh
$ ./main http://example.com:80 /index.html
```
You can also pass a parameter without a protocol or(and) port (by default, the HTTP is used, port 80).


- HTTPS
```sh
$ ./main https://www.boost.com:443 /LICENSE_1_0.txt
```
The port can be omitted (default port is 443). The protocol must be specified.

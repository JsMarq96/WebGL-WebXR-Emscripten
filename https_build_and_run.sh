cd build

twistd -no web --path=. --https=505 -c localhost.pem

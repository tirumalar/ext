openssl pkcs12 -export -in $1.crt -inkey $1.key -out $1.pfx -password pass:eyelock; chmod 644 $1.pfx;

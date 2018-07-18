openssl x509 -req -in $1 -CA $2 -CAkey $3 -CAcreateserial -out $4 -days $5

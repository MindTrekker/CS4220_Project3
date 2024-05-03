# CS4220_Project3
Contributors: Grayson Mckenzie, Issac Laffely

Build instructions:

    For testing purposes, server.c and client.c should be kept in the same directory

    Make sure to include library args:
    ```gcc -o aesServer server.c -lssl -lcrypto```
    ```gcc -o aesClient client.c -lssl -lcrypto```

Running:

    Server: 
        ```./aesServer``` 
        [NO ARGS]
        The server must be closed manually with ```ctrl-c```.


    Client:
        ```./aesClient localhost <filename>```
        The client side will close automaticly when done.
        The file used in ```<filename>``` must be in the same directory as the compiled server file.

Issues:

    The openssl AES encryption and decryption functions only takes 16 bytes at a time, so we had to create a loop to process the text in chunks.

    Attempts to add padding for text where ```text % 16 !=0``` were unsuccessful. Discrepency between server and client when decrypted on last ```message length % 16``` characters.

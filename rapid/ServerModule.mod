MODULE ServerModule
    CONST string IP_ADDR := "127.0.0.1";
    CONST num PORT := 5000;
    
    VAR socketdev listener_socket;
    VAR socketdev client_socket;
    VAR string received_string;
    VAR bool server_running := TRUE;

    PROC SocketServerTask()
        SocketCreate listener_socket;
        SocketBind listener_socket, IP_ADDR, PORT;
        SocketListen listener_socket;
        
        WHILE server_running DO
            TPWrite "Server listening on port " + NumToStr(PORT, 0) + "...";
            
            SocketAccept listener_socket, client_socket;
            TPWrite "Client connected!";
            
            HandleClient;
            
            SocketClose client_socket;
            TPWrite "Client socket closed. Ready for new connection.";
        ENDWHILE
        
        SocketClose listener_socket;
    ENDPROC
    
    PROC HandleClient()
        VAR bool client_active := TRUE;
        
        WHILE client_active DO
            SocketReceive client_socket \Str := received_string;
            TPWrite "Received: " + received_string;
            
            IF received_string = "EXIT" THEN
                client_active := FALSE;
                TPWrite "Client requested disconnect.";
            ELSE
                parse_message received_string;
                
                SocketSend client_socket \Str := "ACK";
            ENDIF
        ENDWHILE
        ERROR
            IF ERRNO = ERR_SOCK_CLOSED THEN
                TPWrite "Connection lost unexpectedly.";
                client_active := FALSE;
                TRYNEXT;
            ENDIF
    ENDPROC
ENDMODULE
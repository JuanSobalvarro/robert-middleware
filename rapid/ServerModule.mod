MODULE ServerModule
    
    VAR socketdev listener_socket;
    VAR socketdev client_socket;
    
    VAR string received_parts{2}; 
    VAR rawbytes raw_buffer;
    VAR bool server_running := TRUE;

    PROC SocketServerTask()
        SocketClose listener_socket;
        SocketClose client_socket;

        SocketCreate listener_socket;
        SocketBind listener_socket, IP_ADDR, PORT;
        SocketListen listener_socket;
        
        WHILE server_running DO
            TPWrite "Server listening on port " + NumToStr(PORT, 0) + "...";
            SocketAccept listener_socket, client_socket \Time:=WAIT_MAX;
            TPWrite "Client connected!";
            
            HandleClient;
            
            SocketClose client_socket;
            TPWrite "Session ended. Ready for new connection.";
        ENDWHILE
    ERROR
        IF ERRNO = ERR_SOCK_TIMEOUT THEN
            RETRY;
        ELSE
            TPWrite "Socket error. Retrying in 5s...";
            WaitTime 5;
            RETRY;
        ENDIF
    ENDPROC
    
    PROC HandleClient()
        VAR bool client_active := TRUE;
        VAR num bytes_received;
        VAR num minimum;
        
        WHILE client_active DO
            ClearRawBytes raw_buffer;
            
            ! Receive up to 1024 bytes into the raw buffer
            SocketReceive client_socket \RawData := raw_buffer \Time:=WAIT_MAX;
            bytes_received := RawBytesLen(raw_buffer);
            
            IF bytes_received > 0 THEN
                IF bytes_received > 160 THEN
                    TPWrite "Received message too long. Closing connection.";
                    client_active := FALSE;
                    Break;
                ENDIF

                IF bytes_received <= 80 THEN
                    minimum := bytes_received;
                ELSE
                    minimum := 80;
                ENDIF

                UnpackRawBytes raw_buffer, 1, received_parts{1} \ASCII := minimum;
                
                IF bytes_received > 80 THEN
                    UnpackRawBytes raw_buffer, 81, received_parts{2} \ASCII := (bytes_received - 80);
                ELSE
                    received_parts{2} := "";
                ENDIF

                TPWrite "Data received (Raw). Length: " + NumToStr(bytes_received, 0);
                
                IF received_parts{1} = "EXIT" THEN
                    client_active := FALSE;
                ELSE
                    parse_message;
                    SocketSend client_socket \Str := "ACK_RAW_UWU";
                ENDIF
            ELSE
                client_active := FALSE;
            ENDIF
        ENDWHILE
    ERROR
        IF ERRNO = ERR_SOCK_TIMEOUT THEN
            RETRY;
        ELSEIF ERRNO = ERR_SOCK_CLOSED THEN
            TPWrite "Connection closed.";
            client_active := FALSE;
            TRYNEXT;
        ENDIF
    ENDPROC
ENDMODULE
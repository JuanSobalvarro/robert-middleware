MODULE ServerModule
    
    VAR socketdev listener_socket;
    VAR socketdev client_socket;

    VAR rawbytes raw_buffer;
    VAR bool server_running := TRUE;
    VAR bool client_connected := FALSE;

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
        VAR num bytes_received;
        VAR num minimum;
        VAR string command;
        
        WHILE client_connected DO
            ClearRawBytes raw_buffer;
            
            ! Receive up to 1024 bytes into the raw buffer
            SocketReceive client_socket \RawData := raw_buffer \Time:=WAIT_MAX;
            bytes_received := RawBytesLen(raw_buffer);
            
            IF bytes_received > 0 THEN
                parse_message raw_buffer, bytes_received;
            ELSE
                TPWrite "Client disconnected.";
                client_connected := FALSE;
            ENDIF
        ENDWHILE
    ERROR
        IF ERRNO = ERR_SOCK_TIMEOUT THEN
            RETRY;
        ELSEIF ERRNO = ERR_SOCK_CLOSED THEN
            TPWrite "Connection closed.";
            client_connected := FALSE;
            TRYNEXT;
        ENDIF
    ENDPROC

    PROC SendResponse(string status_msg)
        IF SocketGetStatus(client_socket) = SOCKET_CONNECTED THEN
            SocketSend client_socket \Str := status_msg;
        ELSE
            TPWrite "Error: Cannot send response, client socket is not connected.";
        ENDIF
    ENDPROC
ENDMODULE
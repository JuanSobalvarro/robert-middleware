MODULE MainModule
    CONST jointtarget ZERO:=[[0,0,0,0,0,0],[9E+9,9E+9,9E+9,9E+9,9E+9,9E+9]];
    CONST robtarget origin:=[[450.00,0.00,500.00],[4.14816E-8,6.1133E-9,-1,-2.53589E-16],[0,0,-1,0],[9E+9,9E+9,9E+9,9E+9,9E+9,9E+9]];
    CONST string IP_ADDR := "127.0.0.1"; 
    CONST num PORT := 5000;
    
    VAR speeddata move_speed := v100;
    VAR zonedata move_zone := fine;
    
    PROC main()
        TPWrite "Initiating at origin...";
        MoveAbsJ ZERO, v100, fine, tool0;
        
        SocketServerTask;
    ENDPROC
ENDMODULE
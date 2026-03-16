MODULE Module1
    CONST jointtarget Zero:=[[0,0,0,0,0,0],[9E+9,9E+9,9E+9,9E+9,9E+9,9E+9]];
    CONST robtarget p10:=[[450.00,0.00,500.00],[0,0,-1,0],[0,0,-1,0],[9E+9,9E+9,9E+9,9E+9,9E+9,9E+9]];
  
    PROC main()
        !Añada aquí su código
        MoveAbsJ Zero\NoEOffs, v1000, z50, tool0;
    ENDPROC
ENDMODULE
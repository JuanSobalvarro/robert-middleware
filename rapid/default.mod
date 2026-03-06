MODULE Module1
    CONST jointtarget Zero:=[[0,0,0,0,0,0],[9E+9,9E+9,9E+9,9E+9,9E+9,9E+9]];
    CONST robtarget p10:=[[450.00,0.00,500.00],[4.14816E-8,6.1133E-9,-1,-2.53589E-16],[0,0,-1,0],[9E+9,9E+9,9E+9,9E+9,9E+9,9E+9]];
  
    PROC main()
        !Añada aquí su código
        MoveAbsJ Zero\NoEOffs, v1000, z50, tool0;
    ENDPROC
ENDMODULE
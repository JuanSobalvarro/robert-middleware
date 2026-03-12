MODULE ValueModule
    FUNC zonedata value_to_zone(num zone)
        TEST zone
            CASE 0:
                RETURN fine;
            CASE 1:
                RETURN z0;
            CASE 2:
                RETURN z1;
            CASE 3:
                RETURN z5;
            CASE 4:
                RETURN z10;
            CASE 5:
                RETURN z15;
            CASE 6:
                RETURN z20;
            CASE 7:
                RETURN z30;
        ENDTEST 
    ENDFUNC
ENDMODULE
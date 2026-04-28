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
    FUNC num zone_to_value(zonedata zone)
        TEST zone
            CASE fine:
                RETURN 0;
            CASE z0:
                RETURN 1;
            CASE z1:
                RETURN 2;
            CASE z5:
                RETURN 3;
            CASE z10:
                RETURN 4;
            CASE z15:
                RETURN 5;
            CASE z20:
                RETURN 6;
            CASE z30:
                RETURN 7;
        ENDTEST
    ENDFUNC
    FUNC string command_to_string(num cmd)
        TEST cmd
            CASE 0x00:
                RETURN "MoveL";
            CASE 0x01:
                RETURN "MoveJ";
            CASE 0x02:
                RETURN "MoveC";
            CASE 0x03:
                RETURN "MoveAbsJ";
            CASE 0x04:
                RETURN "SetSpeed";
            CASE 0x05:
                RETURN "SetZone";
            CASE 0x06:
                RETURN "Exit";
            CASE 0x07:
                RETURN "PING";
            CASE 0x08:
                RETURN "PINGR";
            CASE 0x09:
                RETURN "ZERO";
            CASE 0x0A:
                RETURN "GETSTATUS";
            DEFAULT:
                RETURN "UNKNOWN";
        ENDTEST
    ENDFUNC
ENDMODULE
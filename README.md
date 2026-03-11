# TODO moves
- MoveJ
- MoveAbsJ
- MoveC
- MoveL

# Windows firewall
```bash
New-NetFirewallRule -DisplayName "RobeRT-RobotStudio-In" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 5000
```
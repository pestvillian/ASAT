List of changes to track progress.

4/17/25 9:55pm. Added TMC2226 stepper driver to schematic and followed the datasheet. left questions in the schematic. main concerns are vmotor filtering, do we need diagnostic pins like pdn_uart, diag and index, and what vsense resistors do we use? there are other smaller questions too. -Dorjee

4/17/25 11:42pm. made some small updates. vmotor basic filtering is two 100n decouple caps and one 100u bulk cap. they recommend low ESR caps, which provide faster response time. however, they also recommend voltage spikes to be delayed, which would imply high ESR caps would be good too? need to do more research
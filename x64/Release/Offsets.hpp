#pragma once

/*
async Offset dumper
improved version of @Ph4nton lol dumper by LeftSpace
Thu May 18 16:58:37 2023
*/

/*13.10.509.8402*/
#define ChampionName 0x38D0	//48 81 C1 ? ? ? ? 48 3B CA 74 ? 48 83 7A ? ? 72 ? 48 8B 12
#define AttackRange 0x16C4	//F3 41 0F 10 83 ? ? ? ? 48 8B 1C 24 48 83 C4
#define oAttackRange 0x16C4	//F3 41 0F 10 83 ? ? ? ? 48 8B 1C 24 48 83 C4
#define BaseAtk 0x167C	//8B 81 ? ? ? ? 48 81 C1 ? ? ? ? 89 42 04 E8 ? ? ? ? 48 8B 4B
#define AiManager 0x4F8	//FF 90 ? ? ? ? 4C 8B B4 24 ? ? ? ? 8B 88 ? ? ? ? 89 4D 14
#define MinimapObjectHud 0x320	//48 8B 88 ? ? ? ? 80 79 ? ? F3 0F 10 41
#define Moving 0x2BC	//0F B6 98 ? ? ? ? 48 8B 07 FF 90 ? ? ? ? 48 8B CF
#define MinimapHudPos 0x60	//F3 0F 58 41 ? F3 0F 5C F8 F3 0F 10 41
#define BoundingRadius 0x0	//48 83 B9 88 ? ? ? ? 48 8B D9 0F 29 74 24

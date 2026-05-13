#include "Game_2.h"
#include "Joystick.h"
#include "LCD.h"
#include "Utils.h"
#include <stdint.h>
#include <stdio.h>

static struct{
    int16_t x;
    int16_t y;
    uint8_t hp;
    uint8_t inv; //avoid double damage
    uint8_t cd;
    uint8_t dying; //HP hits zero
    uint32_t death; //death animation trigger
}player;

void G2_Player_Init(){
    player.x = (int16_t)(G2_LCD_W/2-G2_PLAYER_W/2);
    player.y = (int16_t)(G2_LCD_H-G2_PLAYER_H-20);
    player.hp = G2_PLAYER_HP_MAX;
    player.inv = 0;
    player.cd = 0; //cooldown
    player.dying = 0;
    player.death = 0;
}

//updating gameframes
void G2_Player_Update(const Joystick_t *joy){
    //When player is dying, it's not controlable
    if(player.dying)
        return;

    //tickcountdown timers
    if(player.cd>0){
        player.cd--;
    }
    if(player.inv>0){
        player.inv--;
    }

    //joysick input, it has 8 directions NSEW, NE, NW, SE,SW
    int16_t dx = 0;
    int16_t dy = 0;
    switch(joy->direction){
        case N: dy = -G2_PLAYER_SPEED;
                break;
        case S: dy = G2_PLAYER_SPEED;
                break;
        case E: dx = G2_PLAYER_SPEED;
                break;
        case W: dx = -G2_PLAYER_SPEED;
                break;
        case NE:    dx = G2_PLAYER_SPEED;
                    dy = -G2_PLAYER_SPEED;
                    break;
        case NW:    dx = -G2_PLAYER_SPEED;
                    dy = -G2_PLAYER_SPEED;
                    break;
        case SE:    dx = G2_PLAYER_SPEED;
                    dy = G2_PLAYER_SPEED;
                    break;
        case SW:    dx = -G2_PLAYER_SPEED;
                    dy = G2_PLAYER_SPEED;
                    break;
        default:    break; //no movement in the centre
    }
    player.x += dx;
    player.y += dy;
    
    //clamp to prevent sprite from moving outside
    if(player.x<0){
        player.x = 0;
    }
    if(player.x > (int16_t)(G2_LCD_W-G2_PLAYER_W)){
        player.x = (int16_t)(G2_LCD_W-G2_PLAYER_W);
    }
    if(player.y < (int16_t)G2_TOP){ //top boundary
        player.y = (int16_t)G2_TOP;
    }
    if(player.y > (int16_t)(G2_BOT-G2_PLAYER_H)){ //bottom boundary
        player.y = (int16_t)(G2_BOT-G2_PLAYER_H); //-H to keep player in view
    }
}

//Shooting
uint8_t G2_Player_Shoot(void){
    if(player.dying || player.cd>0){
        return 0;
    }

    int16_t bx = player.x + (G2_PLAYER_W/2) - (G2_BULLET_W/2);
    int16_t by = player.y - G2_BULLET_H;

    if(G2_Bullets_PlayerShoot(bx, by)){
        player.cd = G2_PLAYER_CD;
        return 1;
    }
    return 0;
}

//Draw
void G2_Player_Draw(uint32_t count){
    if(player.dying){
        uint32_t i = count - player.death;
        if(i>10)
            return;
    }
    uint8_t draw_ship = 1;
    if(player.inv > 0){
        if(((count/4)&1u)==0){
            draw_ship = 1;
        }else{
            draw_ship = 0;
        }
    }
    if(draw_ship){
        LCD_Draw_Sprite_Colour((uint16_t)player.x, (uint16_t)player.y, G2_PLAYER_H, G2_PLAYER_W, G2_SPR_PLAYER, G2_CYAN);
        uint8_t Jetfire;
        if((count%4)<1){
            Jetfire = G2_YELLOW;
        }else{
        Jetfire = G2_ORANGE;
        }
        LCD_Draw_Rect((uint8_t)(player.x+5), (uint16_t)(player.y+G2_PLAYER_H), 6, 3, Jetfire, 1);
    }
}

//HP
uint8_t G2_Player_GetHP(void){
    return player.hp;
}

//Damage & accessors
uint8_t G2_Player_Damage(uint8_t oof){
    //Hits when player is invincible after getting hit & when player is dying is ignored
    if(player.dying || player.inv > 0){
        return 0;
    }

    //Subtract HP, prevent negatives
    if (oof >= player.hp){ //damage more than current health = player death(0)
        player.hp = 0;
    }else{
        player.hp = (uint8_t)(player.hp-oof);
    }
    player.inv = G2_PLAYER_INV;
    return 1;
}

uint8_t G2_Player_IsDead(void){
    if(player.hp == 0){
        return 1;
    }
    return 0;
}

AABB G2_Player_GetAABB(void){ //Collision Detection from Utils.h
    AABB box;
    box.x = player.x;
    box.y = player.y;
    box.width = G2_PLAYER_W;
    box.height = G2_PLAYER_H;
    return box;
}

int16_t G2_Player_GetCentreX(void){
    return (player.x + (G2_PLAYER_W/2));
}

int16_t G2_Player_GetCentreY(void){
    return (player.y + (G2_PLAYER_H/2));
}

/* ===== Death Animation ===== */

void G2_Player_Death_Start(void){ 
    //Error Handling, if not it gets triggered again and again
    if(player.dying){ //if already dying, nothing happens
        return;
    }
    player.dying = 1;
}

uint8_t G2_Player_Death(uint32_t count){
    if(!player.dying){
        return 1;
    }
    if(player.death == 0){
        player.death = count;
    }

    uint32_t i = count - player.death;

    //Explosions to simulate death
    int16_t cx = player.x + G2_PLAYER_W/2 - G2_EXPLODE_W/2;
    int16_t cy = player.y + G2_PLAYER_H/2 - G2_EXPLODE_H/2;
    if(i == 0){
        G2_FX_Explosion(cx, cy);
    }else if(i == 6){
        G2_FX_Explosion((int16_t)(cx-6), (int16_t)(cy+2));
    }else if(i == 12){
        G2_FX_Explosion((int16_t)(cx+6), (int16_t)(cy-4));
    }else if(i == 18){
        G2_FX_Explosion((int16_t)(cx-4), (int16_t)(cy-4));
    }else if(i == 24){
        G2_FX_Explosion((int16_t)(cx+4), (int16_t)(cy+4));
    }else if(i == 30){
        G2_FX_Explosion(cx, cy);
    }
 
    if (i >= G2_DEATH) {
        return 1;
    }
    return 0;
}

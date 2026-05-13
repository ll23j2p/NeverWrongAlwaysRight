#include <stdint.h>
#include <stdio.h>
#include "Game_2.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "Utils.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"

extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;  //Buzzer control
extern Joystick_cfg_t joystick_cfg; //Joystick control 

/*-------------------------------------------
    State machine
-------------------------------------------*/
typedef enum{
    G2_STATE_TITLE = 0,
    G2_STATE_PLAY,
    G2_STATE_PAUSE,
    G2_STATE_DYING,
    G2_STATE_GAMEOVER,
    G2_STATE_VICTORY,
    G2_STATE_EXIT,
}G2_State;

static uint32_t s_frame_count = 0;
static uint32_t s_Score = 0;
static uint8_t  s_grace = 0;
static uint32_t s_victory_t = 0;
static uint16_t s_victory_shipx = 0;
static uint16_t s_victory_shipy = 0;
static uint32_t s_victory_score = 0;
static uint16_t s_victory_wave = 0;

#define G2_INPUT_GRACE 9
#define G2_LAST_WAVE 5
//phase boundaries for end screen
#define G2_VIC_FLY_END 50
#define G2_VIC_TITLE_END 80
#define G2_VIC_STATS_END 140

/*-------------------------------------------
    Sound Effects(sfx)
-------------------------------------------*/
static void sfx_blip(uint32_t hz, uint32_t ms){
    buzzer_tone(&buzzer_cfg, hz, 50);
    HAL_Delay(ms);
    buzzer_off(&buzzer_cfg);
}

static void sfx_pew(void){
    sfx_blip(1800, 5);
}
static void sfx_kill(void){
    sfx_blip(400, 20);
}
static void sfx_player_hit(void){
    sfx_blip(200, 40);
}
static void sfx_big_thud(void){
    sfx_blip(130, 60);
}
static void sfx_gameover(void){
    sfx_blip(220, 60);
    sfx_blip(180, 80);
}
static void sfx_pause(void){
    sfx_blip(600, 25);
    sfx_blip(900, 25);
}
static void sfx_win(void){
    sfx_blip(523, 60);
    sfx_blip(659, 60);
    sfx_blip(784, 60);
    sfx_blip(1047, 120);
}

/*-------------------------------------------
    HUD Stats + Info
-------------------------------------------*/
static void draw_hud(void){
    LCD_Draw_Rect(0, 0, G2_LCD_W, G2_HUD_H, G2_NAVY, 1);

    //HP bar
    const uint16_t bar_x = 4;
    const uint16_t bar_y = 4;
    const uint16_t bar_w = 60;
    const uint16_t bar_h = 8;

    //HP Border
    LCD_Draw_Rect(bar_x-1, bar_y-1, bar_w+2, bar_h+2, G2_WHITE, 0);

    //HP Empty
    LCD_Draw_Rect(bar_x, bar_y, bar_w, bar_h, G2_BLACK, 1);

    //HP Filled
    uint8_t hp = G2_Player_GetHP();
    uint16_t fill_w = (uint16_t)((uint32_t)hp*bar_w/G2_PLAYER_HP_MAX);
    uint8_t fill_col;
    if(hp >= 60){
        fill_col = G2_GREEN;
    }else if(hp >= 30){
        fill_col = G2_YELLOW;
    }else{
        fill_col = G2_RED;
    }
    if(fill_w>0){
        LCD_Draw_Rect(bar_x, bar_y, fill_w, bar_h, fill_col, 1);
    }
    LCD_printString("HP", 68, 4, G2_WHITE, 1); //Label
    
    //Score display
    char buf[24];
    sprintf(buf, "SCORE:%06lu", (unsigned long)s_Score);
    LCD_printString(buf, 96, 4, G2_GOLD, 1);
    //Wave display
    sprintf(buf, "WAVE:%d", (int)G2_Enemy_GetWave());
    LCD_printString(buf, 196, 4, G2_CYAN, 1);
}

//Title Screen
static G2_State title(void){
    LCD_Fill_Buffer(G2_BLACK);

    //Background motion even on the title screen
    G2_FX_SpacefieldUpdateDraw();
    G2_FX_SpeedLinesUpdateDraw();

    LCD_printString("Space Escape", 18, 60, G2_CYAN, 2);
    LCD_printString("Mission: Escape. Don't Die.", 24, 60, G2_WHITE, 1);
    //Sprite Decos
    LCD_Draw_Sprite_Colour(110, 120, G2_PLAYER_H, G2_PLAYER_W, G2_SPR_PLAYER, G2_CYAN);
    LCD_Draw_Sprite_Colour(50, 120, G2_ENEMY_H, G2_ENEMY_W, G2_SPR_ENEMY1, G2_MAGENTA);
   
    //Fire out of the jet effects
    uint8_t Jetfire;
    if((s_frame_count%4)<1){ //Y,O,O,O
        Jetfire = G2_YELLOW;
    }else{
        Jetfire = G2_ORANGE;
    }
    LCD_Draw_Rect(115, 136, 6, 3, Jetfire, 1);

    if(((s_frame_count/15)&1)==0){
        LCD_printString("Press BT4 to start game", 32, 170, G2_WHITE, 1);
        LCD_printString("Press BT2 to main menu", 32, 185, G2_WHITE, 1);
    }

    LCD_Refresh(&cfg0);

    if(current_input.btn4_pressed){
        return G2_STATE_PLAY;
    }
    if(current_input.btn2_pressed){
        return G2_STATE_EXIT;
    }
    return G2_STATE_TITLE;
}

static void enter_victory(void){
    AABB pbox = G2_Player_GetAABB();
    s_victory_shipx = pbox.x;
    s_victory_shipy = pbox.y;
    s_victory_score = s_Score;
    s_victory_wave = G2_Enemy_GetWave();
    s_victory_t = 0;
}
//Gameplay = Background+Player+Enemies
static G2_State Gameplay(void){
    //Background
    LCD_Fill_Buffer(G2_BLACK);
    G2_FX_SpeedLinesUpdateDraw();
    G2_FX_SpacefieldUpdateDraw();

    Joystick_t joy;
    Joystick_Read(&joystick_cfg, &joy);

    //Player
    G2_Player_Update(&joy);
    if(current_input.btn4_pressed){
        if(G2_Player_Shoot()){
            sfx_pew();
        }
    }

    //Bullets and enemies update
    G2_Bullets_UpdatePlayer();
    G2_Bullets_UpdateEnemy();
    G2_Enemy_Update();

    //Collisions
    uint8_t kills = G2_Enemy_CheckBulletHits(&s_Score, G2_Enemy_GetWave());
    if(kills>0){
        sfx_kill();
    }

    //Enemy Bullets
    AABB pbox = G2_Player_GetAABB();
    G2_Bullet *ebs = G2_Bullets_GetEnemyPool();
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!ebs[i].active){
            continue;
        }
        AABB bbox = {ebs[i].x, ebs[i].y, G2_BULLET_W, G2_BULLET_H};
        if(AABB_Collides(&bbox, &pbox)){
            ebs[i].active = 0;
            if(G2_Player_Damage(G2_DMG_ENEMY_BUL)) {
                G2_FX_Explosion(pbox.x + G2_PLAYER_W / 2 - G2_EXPLODE_W / 2, pbox.y + G2_PLAYER_H / 2 - G2_EXPLODE_H / 2);
                sfx_player_hit();
            }
        }
    }
 
    //Enemy ship rams
    if(G2_Enemy_CheckPlayerCollision(pbox)){
        if(G2_Player_Damage(G2_DMG_ENEMY_RAM)){
            G2_FX_Explosion(pbox.x + G2_PLAYER_W / 2 - G2_EXPLODE_W / 2, pbox.y + G2_PLAYER_H / 2 - G2_EXPLODE_H / 2);
            sfx_player_hit();
        }
    }
 
    //Wave Progression
    if(G2_Enemy_WaveCleared()){
        if(G2_Enemy_GetWave() >= G2_LAST_WAVE){
            enter_victory();
            sfx_win();
            return G2_STATE_VICTORY;
        }
        G2_Enemy_NextWave();
    }
 
    //foreground
    G2_Bullets_DrawPlayer();
    G2_Bullets_DrawEnemy();
    G2_Enemy_Draw();
    G2_Player_Draw(s_frame_count);
    G2_FX_ExplosionsUpdateDraw();
    draw_hud();
 
    if(current_input.btn5_pressed){
        sfx_pause();
        return G2_STATE_PAUSE;
    }
    //death
    if(current_input.btn2_pressed){
        return G2_STATE_EXIT;
    }
    if(G2_Player_IsDead()){
        G2_Player_Death_Start();
        sfx_big_thud();
        return G2_STATE_DYING;
    }
 
    LCD_Refresh(&cfg0);
    s_frame_count++;
    return G2_STATE_PLAY;
}

//Game Pause
static G2_State Pause(void){
    LCD_Fill_Buffer(G2_BLACK);
    G2_FX_SpacefieldUpdateDraw();
    G2_FX_SpeedLinesUpdateDraw();
    G2_Bullets_DrawPlayer();
    G2_Bullets_DrawEnemy();
    G2_Enemy_Draw();
    G2_Player_Draw(s_frame_count);
    G2_FX_ExplosionsUpdateDraw();
    draw_hud();

    LCD_Draw_Rect(40, 90, 160, 100, G2_BLACK, 1);
    LCD_printString("PAUSED", 80, 108, G2_WHITE, 2);
    //blink
    if(((s_frame_count/15)&1)==0){
        LCD_printString("BT5: RESUME", 60, 150, G2_WHITE, 1);
        LCD_printString("BT2: MAIN MENU", 60, 170, G2_WHITE, 1);
    }
    LCD_Refresh(&cfg0);
    s_frame_count++;

    if(current_input.btn5_pressed){
        sfx_pause();
        return G2_STATE_PLAY;
    }
    if(current_input.btn2_pressed){
        return G2_STATE_EXIT;
    }
    return G2_STATE_PAUSE;
}

//Game Over Screen
static G2_State Gameover(void){
    LCD_Fill_Buffer(G2_BLACK);
    LCD_printString("GAME OVER", 40, 80, G2_WHITE, 4);

    char buf[28];
    sprintf(buf, "SCORE:%06lu", (unsigned long)s_Score);
    LCD_printString(buf, 52, 120, G2_WHITE, 1);
    sprintf(buf, "WAVE:%d", (int)G2_Enemy_GetWave());
    LCD_printString(buf, 60, 135, G2_WHITE, 1);
    LCD_printString("BT4 PLAY AGAIN", 50, 180, G2_WHITE, 1);
    LCD_printString("BT2 MAIN MENU", 50, 200, G2_WHITE, 1);

    LCD_Refresh(&cfg0);

    if(current_input.btn4_pressed){
        return G2_STATE_PLAY;
    }
    if(current_input.btn2_pressed){
        return G2_STATE_EXIT;
    }
    return G2_STATE_GAMEOVER;
}

static G2_State Victory(void){
    LCD_Fill_Buffer(G2_BLACK);

    G2_FX_SpacefieldUpdateDraw();
    G2_FX_SpeedLinesUpdateDraw();

    if(s_victory_t < G2_VIC_FLY_END){
        int16_t y = s_victory_shipy - (int16_t)(s_victory_t * 4);

        if(y > -G2_PLAYER_H){
            LCD_Draw_Sprite_Colour((uint16_t)s_victory_shipx, (uint16_t)y, G2_PLAYER_H, G2_PLAYER_W, G2_SPR_PLAYER, G2_CYAN);

            uint8_t Jetfire;
            if((s_frame_count%4)<1){ //Y,O,O,O
                Jetfire = G2_YELLOW;
            }else{
                Jetfire = G2_ORANGE;
            }
            LCD_Draw_Rect((uint16_t)(s_victory_shipx+4), (uint16_t)(y+G2_PLAYER_H), 8, 5, Jetfire, 1);
        }
        //rocket exhaust
        if((s_victory_t%4)==0){
            G2_FX_Explosion(s_victory_shipx + G2_PLAYER_W/2 - G2_EXPLODE_W/2, y+ G2_PLAYER_H);
        }
    }
    if(s_victory_t >= G2_VIC_FLY_END){
        LCD_printString("MISSION", 60, 70, G2_CYAN, 2);
        LCD_printString("SUCCESSFUL", 60, 100, G2_CYAN, 2);
    }
    if(s_victory_t >= G2_VIC_TITLE_END){
        char buf[32];
        sprintf(buf, "FINAL SCORE: %06lu", (unsigned long)s_victory_score);
        LCD_printString(buf, 60, 145, G2_WHITE, 1);
        sprintf(buf, "WAVE: %d", (int)s_victory_wave);
        LCD_printString(buf, 60, 160, G2_WHITE, 1);
    }
    if(s_victory_t >= G2_VIC_STATS_END){
        if(((s_frame_count/15)&1)==0){
            LCD_printString("BT4: PLAY AGAIN", 60, 195, G2_WHITE, 1);
            LCD_printString("BT2: MAIN MENU", 60, 215, G2_WHITE, 1);
        }
    }
    G2_FX_ExplosionsUpdateDraw();
 
    LCD_Refresh(&cfg0);
    s_frame_count++;
    s_victory_t++;

    //prevents a held BT4 from skipping whole animation
    if(s_victory_t >= G2_VIC_STATS_END){
        if(current_input.btn4_pressed){
            return G2_STATE_PLAY;
        }
        if(current_input.btn2_pressed){
            return G2_STATE_EXIT;
        }
    }
    return G2_STATE_VICTORY;
}

static G2_State run_dying_frame(void){
    //Background
    LCD_Fill_Buffer(G2_BLACK);
    G2_FX_SpacefieldUpdateDraw();
    G2_FX_SpeedLinesUpdateDraw();
 
    G2_Bullets_DrawPlayer();
    G2_Bullets_DrawEnemy();
    G2_Enemy_Draw();
    G2_Player_Draw(s_frame_count);
 
    uint8_t done = G2_Player_Death(s_frame_count); //death anim. Returns 1 when finished
 
    G2_FX_ExplosionsUpdateDraw();
    draw_hud();

    LCD_Refresh(&cfg0);
    s_frame_count++;
 
    if(done){
        sfx_gameover();
        HAL_Delay(400);
        return G2_STATE_GAMEOVER;
    }
 
    //skip if btn2 is pressed
    if(current_input.btn2_pressed){
        return G2_STATE_EXIT;
    }
    return G2_STATE_DYING;
}

//Game restart
static void game_reset(void){
    s_Score = 0;
    s_frame_count = 0;
    s_victory_t = 0;

    G2_Player_Init();
    G2_Bullets_Init();
    G2_Enemy_Init();
    G2_FX_ExplosionsInit();
    G2_FX_SpacefieldInit();
    G2_FX_SpeedLinesInit();
}

//frame rate management
static G2_State frame_step(G2_State(*f)(void)){
    uint32_t t_start = HAL_GetTick();
    Input_Read();

    //supress button input for a few frames
    if(s_grace > 0){
        s_grace--;
        current_input.btn2_pressed = 0;
        current_input.btn3_pressed = 0;
        current_input.btn4_pressed = 0;
        current_input.btn5_pressed = 0;
    }
    G2_State next = f();

    //detect state transistions and set the timer for the next screen
    static G2_State prev_state = G2_STATE_TITLE;
    if(next != prev_state){
        s_grace = G2_INPUT_GRACE;
        prev_state = next;
    }

    uint32_t elapsed = HAL_GetTick() - t_start;
    if(elapsed < G2_FRAME_MS){
        HAL_Delay(G2_FRAME_MS - elapsed);
    }
    return next;
}

MenuState Game2_Run(void)
{
    LCD_Set_Palette(PALETTE_DEFAULT);
    LCD_Fill_Buffer(G2_BLACK);
    LCD_Refresh(&cfg0);
 
    /* Title-screen background motion needs the FX systems running before
       the player has actually started a game.                              */
    G2_FX_SpacefieldInit();
    G2_FX_SpeedLinesInit();
    G2_FX_ExplosionsInit();

    s_grace = G2_INPUT_GRACE;
    G2_State state = G2_STATE_TITLE;
 
    while(state != G2_STATE_EXIT){
        switch(state){
 
        case G2_STATE_TITLE:
            state = frame_step(title);
            if(state == G2_STATE_PLAY){
                game_reset();
            }
            break;

        case G2_STATE_PLAY:
            state = frame_step(Gameplay);
            break;

        case G2_STATE_PAUSE:
            state = frame_step(Pause);
            break;
 
        case G2_STATE_DYING:
            state = frame_step(run_dying_frame);
            break;
 
        case G2_STATE_VICTORY:
            state = frame_step(Victory);
            if(state == G2_STATE_PLAY){
                game_reset();
            }
            break;
        
        case G2_STATE_GAMEOVER:
            state = frame_step(Gameover);
            if(state == G2_STATE_PLAY){
                game_reset();
            }
            break;
 
        case G2_STATE_EXIT:
            break;
        }
    }
 
    buzzer_off(&buzzer_cfg);
    return MENU_STATE_HOME;
}
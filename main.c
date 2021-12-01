#include <stdio.h>
#include <stdlib.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <SFML/OpenGL.h>
#include <SFML/Network.h>
#include <SFML/Audio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define BIT_PER_PIXEL 32
#define WIDTH_CONTAIN(x) x >= 0 && x < SCREEN_WIDTH
#define HEIGHT_CONTAIN(y) y >= 0 && y < SCREEN_HEIGHT
#define SCREEN_CONTAIN(x, y) (WIDTH_CONTAIN(x) && HEIGHT_CONTAIN(y))
#define BLUE_CORNFLOWER (sfColor) { 100, 149, 237 }
#define PI 3.14159265358f
#define SQUARE(x) x * x
#define DISTANCE(x1, y1, x2, y2) sqrt(SQUARE((x2 - x1)) + SQUARE((y2 - y1)))
#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a < b) ? a : b
#define FRAND rand() / (float)RAND_MAX

#define TRUE 1
#define FALSE 0

#define G 9.80665f
#define DELTA_TIME 1 / 60.f
#define MICROSECONDS(x) x * 1000000.f

#define POS(data) (sfVector2f){ data.x, data.y }
#define MAX_PLAYER_LIFE 20
// give a random number between 1 / 8 and 3 / 8 of the screen width
#define RAND_POSITION_PLAYER1 FRAND * SCREEN_WIDTH / 4 + SCREEN_WIDTH / 16
// give a random number between 5 / 8 and 7 / 8 of the screen width
#define RAND_POSITION_PLAYER2 FRAND * SCREEN_WIDTH / 4 + SCREEN_WIDTH * 11 / 16

#define MAX_WIND_SPEED 5.f
// give a random number between -MAX_WIND_SPEED and MAX_WIND_SPEED
#define RAND_WIND_SPEED FRAND * 2 * MAX_WIND_SPEED - MAX_WIND_SPEED
// number of lea showing the wind direction
#define PARTICLE_NB 35
// number of layer of leaves
#define WIND_DEPTH 3
// give the speed of the leaves
// depending on how far they are in the background
#define PARTICLE_SPEED(x) 9.f * (x + 1) / WIND_DEPTH
// give the size of the leaves
// depending on how far they are in the background
#define PARTICLE_RADIUS(x) (x + 1.f) / (float)WIND_DEPTH
#define PARTICLE_SIZE(x) (sfVector2f){ PARTICLE_RADIUS(x), PARTICLE_RADIUS(x) }

// the weight indicate the impact that the wind has on the item
#define BLOW_POWER 4.f
#define KNIFE_WEIGHT 0.5f
#define KNIFE_BLOW 0
#define KNIFE_SPEED_ROTATION 50
#define GRENADE_WEIGHT 0.5f
#define GRENADE_BLOW 50
#define GRENADE_SPEED_ROTATION 5.f
#define ROCKET_WEIGHT 1.f
#define ROCKET_BLOW 100
#define ROCKET_SPEED_ROTATION 0
#define MAX_POWER 1000.f
#define MIN_POWER 100.f
#define POWER_STEP 0.005f
#define MAX_ANGLE -90.f
#define MIN_ANGLE 0.f
#define ANGLE_STEP 0.001f
// the pos where are set the item
// when they should not be visible at the screen
#define INVISIBLE_POS -30.f
#define ANGLE_DIST_X(x) cos(x / 180.f * PI)
#define ANGLE_DIST_Y(x) sin(-x / 180.f * PI)
// distance between the player and the bullet spawn
#define OFF_SET(data, X) 100 * ANGLE_DIST_##X(data)

// set the size of the gauge proportionnal to the shoot power of the player
#define POWER_GAUGE_SIZE(x) { 0, 0 , 100 * x / MAX_POWER, 50 }
// switch the direction of the gauge
#define POWER_GAUGE_LEFT (sfVector2f){ -1.f, 1.f }
#define POWER_GAUGE_RIGHT (sfVector2f){ 1.f, 1.f }
#define POWER_GAUGE_ORIGIN (sfVector2f){ 0.f, 25.f }

// caculate the size of the bar depending on
// how much life does remain to the player
#define LIFE_BAR_SIZE(x) (sfVector2f){ x / (float)MAX_PLAYER_LIFE, 1.f }
#define LIFE_BAR_POS_P1 (sfVector2f){ SCREEN_WIDTH / 12.f + 2.f, 7.f }
#define LIFE_BAR_POS_P2 (sfVector2f){ SCREEN_WIDTH * 7.f / 12.f + 2.f, 7.f }
#define LIFE_BAR_BOUNDS_POS_P1 (sfVector2f){ SCREEN_WIDTH / 12.f, 5.f }
#define LIFE_BAR_BOUNDS_POS_P2 (sfVector2f){ SCREEN_WIDTH * 7.f / 12.f, 5.f }

#define CHARACTER_SIZE 40
#define INVENTORY_POSITION_X SCREEN_WIDTH / 2 - 270
#define INVENTORY_POSITION_Y SCREEN_HEIGHT / 2 - 202
#define INVENTORY_GRENADE_POSITION_X INVENTORY_POSITION_X + 70
#define INVENTORY_GRENADE_POSITION_Y INVENTORY_POSITION_Y + 70
#define INVENTORY_ROCKET_POSITION_X INVENTORY_POSITION_X + 70
#define INVENTORY_ROCKET_POSITION_Y INVENTORY_POSITION_Y + 200
#define INVENTORY_KNIFE_POSITION_X INVENTORY_POSITION_X + 70
#define INVENTORY_KNIFE_POSITION_Y INVENTORY_POSITION_Y + 334
#define INVENTORY_TEXT_POSITION_X INVENTORY_POSITION_X + 140
#define INVENTORY_TEXT_POSITION_Y INVENTORY_POSITION_Y + 16
#define GRENADE_NAME "Grenade\n\n\n"
#define GRENADE_SENSITIVITY "Moyen sensible au vent.\n"
#define GRENADE_POWER "Eplosion moyenne.\n\n"
#define GRENADE_DESCRIPTION "... Ouais une arme moyenne\nquoi."
#define ROCKET_NAME "Bazooka\n\n\n"
#define ROCKET_SENSITIVITY "Peu sensible au vent.\n"
#define ROCKET_POWER "grosse explosion.\n\n"
#define ROCKET_DESCRIPTION "Un grand classique."
#define KNIFE_NAME "Couteau\n\n\n"
#define KNIFE_SENSITIVITY "Tres sensible au vent.\n"
#define KNIFE_POWER "pas d'explosion.\n\n"
#define KNIFE_DESCRIPTION "Detruisez la dignite de votre adversaire."

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// Enum /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum Turn
{
    TURN_PLAYER1,
    TURN_PLAYER2,
    END_TURN
} Turn;

typedef enum ProjectileType
{
    P_GRENADE = 1,
    P_ROCKET = 2,
    P_KNIFE = 3
} ProjectileType;

typedef enum SpriteName
{
    SP_BACKGROUND,
    SP_BATTLE_FIELD,
    SP_TURRET_P1,
    SP_TURRET_P2,
    SP_INVENTORY,
    SP_SELECTOR,
    SP_GRENADE,
    SP_ROCKET,
    SP_KNIFE,
    SP_MIRE,
    SP_LEAF,
    SP_LIFE_BAR,
    SP_LIFE_BAR_BOUNDS,
    SP_GAUGE,
    SP_COUNT,
} SpriteName;

typedef enum SoundName
{
    SD_HURT,
    SD_COUNT,
} SoundName;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////// Structures //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Turret
{
    float x;
    float y;
    float horizontalSpeed;
    float verticalSpeed;
    char life;
    float shootAngle;
    float shootPower;
    ProjectileType projectileType;
} Turret;

typedef struct Projectile
{
    float x;
    float y;
    float horizontalSpeed;
    float verticalSpeed;
    char isLaunch;
    ProjectileType type;
} Projectile;

typedef struct Game
{
    Turret player1;
    Turret player2;
    Projectile projectile;
    // for the game to be regular, we limite the elapsed time between each frame
    sfTime timePerFrame;
    float windSpeed;
    sfVector2f windParticules[WIND_DEPTH][PARTICLE_NB];
    Turn turn;
    int isInventoryOpen;
} Game;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////// Utility fonction ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// return the corresponding sprite to a projectile
SpriteName GetSprite(ProjectileType _type)
{
    switch (_type) {
        case P_GRENADE:
            return SP_GRENADE;
        
        case P_ROCKET:
            return SP_ROCKET;
            
        case P_KNIFE:
            return SP_KNIFE;
            
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

sfSound* LoadSound(char* _path)
{
    sfSoundBuffer* soundBuffer = sfSoundBuffer_createFromFile(_path);
    sfSound* sound = sfSound_create();
    sfSound_setBuffer(sound, soundBuffer);
    
    return sound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

sfSprite* LoadSprite(char* _path, int _isCentered)
{
    sfTexture* texture = sfTexture_createFromFile(_path, NULL);
    sfSprite* sprite = sfSprite_create();
    sfSprite_setTexture(sprite, texture, TRUE);
    
    if (_isCentered)
    {
        sfFloatRect rect = sfSprite_getLocalBounds(sprite);
        sfSprite_setOrigin(sprite, (sfVector2f) { rect.width / 2, rect.height / 2 });
    }
    
    return sprite;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// draw the sprite at the position ( _x, _y )
// with a rotation of _angle degree in the window
void BlitSprite(sfSprite* _sprite, float _x, float _y,
                float _angle, sfRenderWindow* _window)
{
    sfSprite_setPosition(_sprite, (sfVector2f) { _x, _y });
    sfSprite_setRotation(_sprite, _angle);
    sfRenderWindow_drawSprite(_window, _sprite, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// draw the Text at the position ( _x, _y )
// with a rotation of _angle degree in the window
void BlitText(sfText* _text, float _x, float _y,
              float _angle, sfRenderWindow* _window)
{
    sfText_setPosition(_text, (sfVector2f) { _x, _y });
    sfText_setRotation(_text, _angle);
    sfRenderWindow_drawText(_window, _text, NULL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// check if the given coordinates ( _x, _y) are inside the image
// then set the pixel color to _color
int DrawPixel(sfImage* _img, int _x, int _y, sfColor _color)
{
    if (_x >= 0 && _x < (int)sfImage_getSize(_img).x
        && _y >= 0 && _y < (int)sfImage_getSize(_img).y)
    {
        sfImage_setPixel(_img, _x, _y, _color);
        // work as a boolean, the function return 1
        // to tell that the pixel was inside the image
        return TRUE;
    }
    
    // return 0 if the image isn't in the image
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// Game fonction /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Game Initialize(Game _game, sfImage* _fieldImg, sfTexture* _fieldTexture)
{
    _game.turn = TURN_PLAYER1;
    _game.isInventoryOpen = FALSE;
    
    _game.player1.life = MAX_PLAYER_LIFE;
    // we don't want the player to always start at the same position.
    // we initialize it randomly beetwen 1 / 8
    // and 3 / 8 from the left side of the screen
    _game.player1.x = RAND_POSITION_PLAYER1;
    // the player start at the top of the screen then fall to the grounds
    _game.player1.y = 0.f;
    _game.player1.verticalSpeed = 0.f;
    _game.player1.horizontalSpeed = 0.f;
    _game.player1.shootPower = MIN_POWER;
    _game.player1.shootAngle =  MAX_ANGLE / 2;
    // set default projectile to ROCKET
    _game.player1.projectileType = P_ROCKET;
    
    _game.player2.life = MAX_PLAYER_LIFE;
    // we initialize it randomly beetwen 5 / 8
    // and 7 / 8 from the left side of the screen
    _game.player2.x = RAND_POSITION_PLAYER2;
    // the player start at the top of the screen then fall to the grounds
    _game.player2.y = 0.f;
    _game.player2.verticalSpeed = 0.f;
    _game.player2.horizontalSpeed = 0.f;
    _game.player2.shootPower = MIN_POWER;
    _game.player2.shootAngle = MAX_ANGLE / 2;
    // set default projectile to ROCKET
    _game.player2.projectileType = P_ROCKET;
    
    // we place the projectile outside the window
    // not to see it while no one has shoot
    _game.projectile.x = INVISIBLE_POS;
    _game.projectile.y = INVISIBLE_POS;
    // we don't want the projectile to sudently appear
    // from nowhere so we initialize it's speed to 0
    _game.projectile.horizontalSpeed = 0.f;
    _game.projectile.verticalSpeed = 0.f;
    // decide if the projectile must be drawn or not
    _game.projectile.isLaunch = FALSE;
    _game.projectile.type = P_ROCKET;
    
    // the wind speed is initialize beetwen -5.f and 5.f
    _game.windSpeed = RAND_WIND_SPEED;

    // creates particules that shown the wind direction
    // we place them randomly on the stage
    for (int x = 0; x < WIND_DEPTH; x++)
    {
        for (int y = 0; y < PARTICLE_NB; y++)
        {
            sfVector2f randomPosition = { FRAND * SCREEN_WIDTH, FRAND * SCREEN_HEIGHT};
            _game.windParticules[x][y] = randomPosition;
        }
    }
    
    return _game;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Game Input(Game _game, sfRenderWindow* _window,
           sfImage* _fieldImg, sfTexture* _fieldTexture)
{
    sfEvent event;
    while (sfRenderWindow_pollEvent(_window, &event))
    {
        // closes the window if requested
        if (event.type == sfEvtClosed) sfRenderWindow_close(_window);

		if (event.type == sfEvtMouseButtonPressed)
		{
			printf("%d %d\n", sfMouse_getPositionRenderWindow(_window).x, sfMouse_getPositionRenderWindow(_window).y);
		}

        // handle unique event
        // players can only play if the projectile isn't launch
        if (!_game.projectile.isLaunch)
        {
            if (event.type == sfEvtKeyPressed && _game.turn != END_TURN)
            {
                switch (event.key.code)
                {
                    case sfKeyI:
                        // open or close the inventary depending on his current state
                        _game.isInventoryOpen = !_game.isInventoryOpen;
                        // if we close the inventary we update the selected weapon
                        if (!_game.isInventoryOpen)
                        {
                            switch (_game.turn)
                            {
                                case TURN_PLAYER1:
                                    _game.player1.shootPower = MIN_POWER;
                                    _game.player1.projectileType = _game.projectile.type;
                                    break;
                                    
                                case TURN_PLAYER2:
                                    _game.player2.shootPower = MIN_POWER;
                                    _game.player2.projectileType = _game.projectile.type;
                                    break;
                                    
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            switch (_game.turn)
                            {
                                case TURN_PLAYER1:
                                    _game.projectile.type = _game.player1.projectileType;
                                    break;
                                    
                                case TURN_PLAYER2:
                                    _game.projectile.type = _game.player2.projectileType;
                                    break;
                                    
                                default:
                                    break;
                            }
                        }
                        break;
                        
                        // change the selected weapon when inventory is open
                    case sfKeyUp:
                        if (_game.isInventoryOpen)
                        {
                            _game.projectile.type = MAX(_game.projectile.type - 1, P_GRENADE);
                        }
                        break;
                        
                    case sfKeyDown:
                        if (_game.isInventoryOpen)
                        {
                            _game.projectile.type = MIN(_game.projectile.type + 1, P_KNIFE);
                        }
                        break;
                        
                    default:
                        break;
                }
            }
            
            if(event.type == sfEvtKeyReleased
               && event.key.code == sfKeySpace
               && !_game.isInventoryOpen)
            {
                switch(_game.turn)
                {
                    case TURN_PLAYER2:
                        // if the player released the space button we place the projectile
                        // near the player
                        _game.projectile.x = _game.player2.x - OFF_SET(_game.player2.shootAngle, X);
                        _game.projectile.y = _game.player2.y - OFF_SET(_game.player2.shootAngle, Y);
                        // gives the projectile the power and angle that the player decided
                        _game.projectile.horizontalSpeed = ANGLE_DIST_X(_game.player2.shootAngle)
                                                            * -_game.player2.shootPower * DELTA_TIME;
                        _game.projectile.verticalSpeed = ANGLE_DIST_Y(_game.player2.shootAngle)
                                                            * -_game.player2.shootPower * DELTA_TIME;
                        _game.projectile.type = _game.player2.projectileType;
                        // launch the projectile, while it's true the players won't be able to play
                        _game.projectile.isLaunch = TRUE;
                        _game.turn = TURN_PLAYER1;
                        // reset the shoot power for the next turn
                        _game.player1.shootPower = MIN_POWER;
                        break;
                        
                    case TURN_PLAYER1:
                        // if the player released the space button we place the projectile
                        // near the player
                        _game.projectile.x = _game.player1.x + OFF_SET(_game.player1.shootAngle, X);
                        _game.projectile.y = _game.player1.y - OFF_SET(_game.player1.shootAngle, Y);
                        // gives the projectile the power and angle that the player decided
                        _game.projectile.horizontalSpeed = ANGLE_DIST_X(_game.player1.shootAngle)
                                                            * _game.player1.shootPower * DELTA_TIME;
                        _game.projectile.verticalSpeed = ANGLE_DIST_Y(_game.player1.shootAngle)
                                                            * -_game.player1.shootPower * DELTA_TIME;
                        _game.projectile.type = _game.player1.projectileType;
                        // launch the projectile, while it's true the players won't be able to play
                        _game.projectile.isLaunch = TRUE;
                        _game.turn = TURN_PLAYER2;
                        // reset the shoot power for the next turn
                        _game.player2.shootPower = MIN_POWER;
                        break;
                        
                        // when the game is over pressing space relaunch a new game
                    case END_TURN:
                        _game = Initialize(_game, _fieldImg, _fieldTexture);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    // again players can only play when the projectile isn't moving
    // and when the inventory is closed
    if (!_game.projectile.isLaunch && !_game.isInventoryOpen)
    {
        // key up allow to increase the shoot angle
        if (sfKeyboard_isKeyPressed(sfKeyUp))
        {
            switch (_game.turn)
            {
                case TURN_PLAYER2:
                    // we don't want the angle to go over the MAX_ANGLE
                    _game.player2.shootAngle =
                            MAX(_game.player2.shootAngle - ANGLE_STEP, MAX_ANGLE);
                    break;
                    
                case TURN_PLAYER1:
                    _game.player1.shootAngle =
                            MAX(_game.player1.shootAngle - ANGLE_STEP, MAX_ANGLE);
                    break;
                    
                default:
                    break;
            }
        }
        
        // key down allow to decrease the shoot angle
        if (sfKeyboard_isKeyPressed(sfKeyDown))
        {
            switch (_game.turn)
            {
                case TURN_PLAYER2:
                    // we don't want the angle to go under the MIN_ANGLE
                    _game.player2.shootAngle =
                            MIN(_game.player2.shootAngle + ANGLE_STEP, MIN_ANGLE);
                    break;
                    
                case TURN_PLAYER1:
                    _game.player1.shootAngle =
                            MIN(_game.player1.shootAngle + ANGLE_STEP, MIN_ANGLE);
                    break;
                    
                default:
                    break;
            }
        }
        
        // while the space button is pressed the shoot power is increased
        if (sfKeyboard_isKeyPressed(sfKeySpace))
        {
            switch (_game.turn)
            {
                case TURN_PLAYER2:
                    // we don't want the shoot power to go over the MAX_POWER
                    _game.player2.shootPower =
                            MIN(_game.player2.shootPower + POWER_STEP, MAX_POWER);
                    break;
                    
                case TURN_PLAYER1:
                    _game.player1.shootPower =
                            MIN(_game.player1.shootPower + POWER_STEP, MAX_POWER);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    return _game;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Game Update(Game _game, sfImage* _fieldImg,
            sfTexture* _fieldTexture, sfSprite* _sprites[], sfSound* _sounds[])
{
    // we store the bounds of the turret's sprites to uses them as hitbox
    // so we need to update there position first
    sfSprite_setPosition(_sprites[SP_TURRET_P1], POS(_game.player1));
    sfFloatRect player1Bounds = sfSprite_getGlobalBounds(_sprites[SP_TURRET_P1]);
    sfSprite_setPosition(_sprites[SP_TURRET_P2], POS(_game.player2));
    sfFloatRect player2Bounds = sfSprite_getGlobalBounds(_sprites[SP_TURRET_P2]);
    // ditto with the projectile
    sfSprite_setPosition(_sprites[GetSprite(_game.projectile.type)],
                         POS(_game.projectile));
    sfFloatRect projectileBounds =
            sfSprite_getGlobalBounds(_sprites[GetSprite(_game.projectile.type)]);
    
    // change the position of the mire
    // depending on who is firing and what is his shoot angle
    switch (_game.turn)
    {
        case TURN_PLAYER2:
        {
            sfVector2f pos;
            pos.x = _game.player2.x - OFF_SET(_game.player2.shootAngle, X);
            pos.y = _game.player2.y - OFF_SET(_game.player2.shootAngle, Y);
            sfSprite_setPosition(_sprites[SP_MIRE], pos);
        }
            break;
            
        case TURN_PLAYER1:
        {
            sfVector2f pos;
            pos.x = _game.player1.x + OFF_SET(_game.player1.shootAngle, X);
            pos.y = _game.player1.y - OFF_SET(_game.player1.shootAngle, Y);
            sfSprite_setPosition(_sprites[SP_MIRE], pos);
        }
            break;
            
        default:
            break;
    }
    
    // applys gravity and wind to the particules
    // the impact of thoses depend on how deep is the layer
    for (int x = 0; x < WIND_DEPTH; x++)
    {
        for (int y = 0; y < PARTICLE_NB; y++)
        {
            _game.windParticules[x][y].x += PARTICLE_SPEED(x)
                                            * _game.windSpeed * DELTA_TIME;
            // moves the particules across the screen when it reachs an edge
            if (_game.windParticules[x][y].x >= SCREEN_WIDTH)
            {
                _game.windParticules[x][y].x = 0;
            }
            if (_game.windParticules[x][y].x < 0)
            {
                _game.windParticules[x][y].x = SCREEN_WIDTH - 1;
            }
            
            _game.windParticules[x][y].y += PARTICLE_SPEED(x) * G * DELTA_TIME;
            // when the particules reach the bottom of the screen the are teleported
            // to the top with a new random x position
            if (_game.windParticules[x][y].y >= SCREEN_HEIGHT)
            {
                _game.windParticules[x][y].x = FRAND * SCREEN_WIDTH;
                _game.windParticules[x][y].y = 0;
            }
        }
    }
    
    // apply gravity for both player
    _game.player1.verticalSpeed += G * DELTA_TIME;
    _game.player2.verticalSpeed += G * DELTA_TIME;
    if (_game.projectile.isLaunch)
    {
        // apply gravity to the projectile
        _game.projectile.verticalSpeed += G * DELTA_TIME;
        // apply the wind force to the projectile
        switch (_game.projectile.type)
        {
            case P_GRENADE:
                _game.projectile.horizontalSpeed +=
                        _game.windSpeed / GRENADE_WEIGHT * DELTA_TIME;
                break;
                
            case P_ROCKET:
                _game.projectile.horizontalSpeed +=
                        _game.windSpeed / ROCKET_WEIGHT * DELTA_TIME;
                break;
                
            case P_KNIFE:
                _game.projectile.horizontalSpeed +=
                        _game.windSpeed / KNIFE_WEIGHT * DELTA_TIME;
                break;
                
            default:
                break;
        }
    }
    
    // if the players are in contact with the ground
	// there vertical speed are reset
    if (SCREEN_CONTAIN(_game.player1.x, _game.player1.y + player1Bounds.height / 2)
        && sfImage_getPixel(_fieldImg, (int)(_game.player1.x),
                            (int)(_game.player1.y + player1Bounds.height / 2)).a > 0)
    {
        _game.player1.verticalSpeed = 0.f;
    }
    
    if (SCREEN_CONTAIN(_game.player2.x, _game.player2.y + player2Bounds.height / 2)
        && sfImage_getPixel(_fieldImg, (int)(_game.player2.x),
                            (int)(_game.player2.y + player2Bounds.height / 2)).a > 0)
    {
        _game.player2.verticalSpeed = 0.f;
    }
    
    // if the projectile touch the ground it digs a hole of ROCKET_BLOW radius
    if (SCREEN_CONTAIN(_game.projectile.x, _game.projectile.y)
        && sfImage_getPixel(_fieldImg,
                            (int)(_game.projectile.x), (int)(_game.projectile.y)).a > 0)
    {
        int rayon;
        switch (_game.projectile.type)
        {
            case P_GRENADE:
                rayon = GRENADE_BLOW;
                break;
                
            case P_ROCKET:
                rayon = ROCKET_BLOW;
                break;
                
            case P_KNIFE:
                rayon = KNIFE_BLOW;
                break;
                
            default:
                break;
        }
        
        for (int x = -rayon; x < rayon; x++)
        {
            for (int y = -sqrt(rayon * rayon - x * x);
                 y < sqrt(rayon * rayon - x * x); y++)
            {
                DrawPixel(_fieldImg, (int)(_game.projectile.x) + x,
                          (int)(_game.projectile.y) + y, sfTransparent);
            }
        }
        sfTexture_updateFromImage(_fieldTexture, _fieldImg, 0, 0);
        
        // if a player is standing inside the blow radius he takes proportionnal damage
        // to his distance with the projectile
        float distance =
            DISTANCE(_game.projectile.x, _game.projectile.y,
                     _game.player1.x, _game.player1.y);
        if (distance <= rayon + player1Bounds.height / 2)
        {
            _game.player1.life =
                MAX(_game.player1.life - MAX_PLAYER_LIFE * 
				(1.f - distance / (rayon + player2Bounds.height / 2)), 0);
            sfSound_play(_sounds[SD_HURT]);
            if (_game.player1.life == 0)
            {
                _game.turn = END_TURN;
            }
        }
        
        distance =
            DISTANCE(_game.projectile.x, _game.projectile.y,
                     _game.player2.x, _game.player2.y);
        if (distance <= rayon + player2Bounds.height / 2)
        {
            _game.player2.life =
                MAX(_game.player2.life - MAX_PLAYER_LIFE * 
				(1.f - distance / (rayon + player2Bounds.height / 2)), 0);
            sfSound_play(_sounds[SD_HURT]);
            if (_game.player2.life == 0)
            {
                _game.turn = END_TURN;
            }
        }
        
        // then we replace the projectile out of the screen not the player
        // can see it and we reset its speed
        _game.projectile.verticalSpeed = 0.f;
        _game.projectile.horizontalSpeed = 0.f;
        _game.projectile.x = INVISIBLE_POS;
        _game.projectile.y = INVISIBLE_POS;
        _game.projectile.isLaunch = FALSE;
        // change the power of the wind
        _game.windSpeed = RAND_WIND_SPEED;
    }
    
    // check if the projectile directly touch the player
    // or if the player get out of the screen limits
    // in thoses case we kill the player
    if (sfFloatRect_intersects(&player1Bounds, &projectileBounds, NULL)
        || !(SCREEN_CONTAIN(_game.player1.x, _game.player1.y)))
    {
        _game.player1.life = 0;
        _game.projectile.verticalSpeed = 0.f;
        _game.projectile.horizontalSpeed = 0.f;
        _game.projectile.x = INVISIBLE_POS;
        _game.projectile.y = INVISIBLE_POS;
        _game.projectile.isLaunch = FALSE;
        _game.windSpeed = RAND_WIND_SPEED;
        _game.turn = END_TURN;
        sfSound_play(_sounds[SD_HURT]);
    }
    
    if (sfFloatRect_intersects(&player2Bounds, &projectileBounds, NULL)
        || !(SCREEN_CONTAIN(_game.player2.x, _game.player2.y)))
    {
        _game.player2.life = 0;
        _game.projectile.verticalSpeed = 0.f;
        _game.projectile.horizontalSpeed = 0.f;
        _game.projectile.x = INVISIBLE_POS;
        _game.projectile.y = INVISIBLE_POS;
        _game.projectile.isLaunch = FALSE;
        _game.windSpeed = RAND_WIND_SPEED;
        _game.turn = END_TURN;
        sfSound_play(_sounds[SD_HURT]);
    }
    
    // check if the projectile is still in the screen.
    // otherwise we reset it
    if (_game.projectile.isLaunch)
    {
        if (!(_game.projectile.x >= 0
              && _game.projectile.x < SCREEN_WIDTH
              && _game.projectile.y < SCREEN_WIDTH))
        {
            _game.projectile.verticalSpeed = 0.f;
            _game.projectile.horizontalSpeed = 0.f;
            _game.projectile.x = INVISIBLE_POS;
            _game.projectile.y = INVISIBLE_POS;
            _game.projectile.isLaunch = FALSE;
            _game.windSpeed = RAND_WIND_SPEED;
        }
    }
    
    // moves players and projectile depending on there speed
    _game.projectile.x += _game.projectile.horizontalSpeed;
    _game.projectile.y += _game.projectile.verticalSpeed;
    _game.player1.x += _game.player1.horizontalSpeed;
    _game.player1.y += _game.player1.verticalSpeed;
    _game.player2.x += _game.player1.horizontalSpeed;
    _game.player2.y += _game.player2.verticalSpeed;
    
    return _game;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Display(Game _game, sfRenderWindow* _window,
             sfSprite* _sprites[], sfText* _text)
{
    // first we clear the previous frame
    sfRenderWindow_clear(_window, BLUE_CORNFLOWER);
    // draw the background first
    BlitSprite(_sprites[SP_BACKGROUND], 0, 0, 0, _window);
    // draw the first two deeper wind particules layer
    for (int x = 0; x < WIND_DEPTH - 1; x++)
    {
        for (int y = 0; y < PARTICLE_NB; y++)
        {
            sfSprite_setScale(_sprites[SP_LEAF], PARTICLE_SIZE(x));
            sfSprite_setPosition(_sprites[SP_LEAF], _game.windParticules[x][y]);
            sfRenderWindow_drawSprite(_window, _sprites[SP_LEAF], NULL);
            sfSprite_setScale(_sprites[SP_LEAF], PARTICLE_SIZE(x));
            sfSprite_setPosition(_sprites[SP_LEAF], _game.windParticules[x][y]);
            sfRenderWindow_drawSprite(_window, _sprites[SP_LEAF], NULL);
        }
    }
    
    // draw the battlefield
    BlitSprite(_sprites[SP_BATTLE_FIELD], 0, 0, 0, _window);
    
    // if players are still alive we draw them
    if (_game.player1.life > 0)
    {
        sfRenderWindow_drawSprite(_window, _sprites[SP_TURRET_P1], NULL);
    }
    if (_game.player2.life > 0)
    {
        sfRenderWindow_drawSprite(_window, _sprites[SP_TURRET_P2], NULL);
    }
    
    // draw the corresponding projectile
    switch (_game.projectile.type)
    {
        case P_GRENADE:
            sfSprite_rotate(_sprites[SP_GRENADE], GRENADE_SPEED_ROTATION);
            sfSprite_setPosition(_sprites[SP_GRENADE], POS(_game.projectile));
            sfRenderWindow_drawSprite(_window, _sprites[SP_GRENADE], NULL);
            break;
            
        case P_ROCKET:
            sfSprite_rotate(_sprites[SP_ROCKET], ROCKET_SPEED_ROTATION);
            sfSprite_setPosition(_sprites[SP_ROCKET], POS(_game.projectile));
            sfRenderWindow_drawSprite(_window, _sprites[SP_ROCKET], NULL);
            break;
            
        case P_KNIFE:
            sfSprite_rotate(_sprites[SP_KNIFE], KNIFE_SPEED_ROTATION);
            sfSprite_setPosition(_sprites[SP_KNIFE], POS(_game.projectile));
            sfRenderWindow_drawSprite(_window, _sprites[SP_KNIFE], NULL);
            break;
            
        default:
            break;
    }
    
    // draw the power gauge only if the player is pressing space during is turn
    if (!_game.projectile.isLaunch
        && !_game.isInventoryOpen
        && sfKeyboard_isKeyPressed(sfKeySpace))
    {
        switch (_game.turn)
        {
            case TURN_PLAYER1:
            {
                // turn the power gauge sprite so it's in the same direction as the player
                sfSprite_setScale(_sprites[SP_GAUGE], POWER_GAUGE_RIGHT);
                // set the size of the rect proportionnal to the power of the shoot
                sfIntRect rect = POWER_GAUGE_SIZE(_game.player1.shootPower);
                sfSprite_setTextureRect(_sprites[SP_GAUGE], rect);
                BlitSprite(_sprites[SP_GAUGE], _game.player1.x,
                           _game.player1.y, _game.player1.shootAngle, _window);
            }
                break;
                
            case TURN_PLAYER2:
            {
                // turn the power gauge sprite so it's in the same direction as the player
                sfSprite_setScale(_sprites[SP_GAUGE], POWER_GAUGE_LEFT);
                // set the size of the rect proportionnal to the power of the shoot
                sfIntRect rect = POWER_GAUGE_SIZE(_game.player2.shootPower);
                sfSprite_setTextureRect(_sprites[SP_GAUGE], rect);
                BlitSprite(_sprites[SP_GAUGE], _game.player2.x,
                           _game.player2.y, -_game.player2.shootAngle, _window);
            }
                break;
                
            default:
                break;
        }
    }

    if (_game.turn != END_TURN)
    {
        sfRenderWindow_drawSprite(_window, _sprites[SP_MIRE], NULL);
    }
    
    if (_game.isInventoryOpen)
    {
        BlitSprite(_sprites[SP_INVENTORY],
                   INVENTORY_POSITION_X, INVENTORY_POSITION_Y, 0.f, _window);
        BlitSprite(_sprites[SP_GRENADE],
                   INVENTORY_GRENADE_POSITION_X, INVENTORY_GRENADE_POSITION_Y, 0.f, _window);
        BlitSprite(_sprites[SP_ROCKET],
                   INVENTORY_ROCKET_POSITION_X, INVENTORY_ROCKET_POSITION_Y, 0.f, _window);
        BlitSprite(_sprites[SP_KNIFE],
                   INVENTORY_KNIFE_POSITION_X, INVENTORY_KNIFE_POSITION_Y, 0.f, _window);
        sfText_setOrigin(_text, (sfVector2f){ 0.f, 0.f });
        switch (_game.projectile.type)
        {
            case P_GRENADE:
            {
                char string[100];
                strcpy(string, GRENADE_NAME);
                strcat(string, GRENADE_SENSITIVITY);
                strcat(string, GRENADE_POWER);
                strcat(string, GRENADE_DESCRIPTION);
                sfText_setString(_text,string);
                BlitSprite(_sprites[SP_SELECTOR],
                           INVENTORY_GRENADE_POSITION_X, INVENTORY_GRENADE_POSITION_Y, 0.f, _window);
            }
                break;
                
            case P_ROCKET:
            {
                char string[100];
                strcpy(string, ROCKET_NAME);
                strcat(string, ROCKET_SENSITIVITY);
                strcat(string, ROCKET_POWER);
                strcat(string, ROCKET_DESCRIPTION);
                sfText_setString(_text,string);
                BlitSprite(_sprites[SP_SELECTOR],
                           INVENTORY_ROCKET_POSITION_X, INVENTORY_ROCKET_POSITION_Y, 0.f, _window);
            }
                break;
                
            case P_KNIFE:
            {
                char string[150];
                strcpy(string, KNIFE_NAME);
                strcat(string, KNIFE_SENSITIVITY);
                strcat(string, KNIFE_POWER);
                strcat(string, KNIFE_DESCRIPTION);
                sfText_setString(_text,string);
                BlitSprite(_sprites[SP_SELECTOR],
                           INVENTORY_KNIFE_POSITION_X, INVENTORY_KNIFE_POSITION_Y, 0.f, _window);
            }
                break;
                
            default:
                break;
        }
        BlitText(_text,
                 INVENTORY_TEXT_POSITION_X, INVENTORY_TEXT_POSITION_Y, 0.f, _window);
    }

    // draw the final wind particules layer
    for (int x = 0; x < PARTICLE_NB; x++)
    {
        sfSprite_setScale(_sprites[SP_LEAF],
                          PARTICLE_SIZE(WIND_DEPTH - 1));
        sfSprite_setPosition(_sprites[SP_LEAF],
                             _game.windParticules[WIND_DEPTH - 1][x]);
        sfRenderWindow_drawSprite(_window, _sprites[SP_LEAF], NULL);
    }
    
    // set the position, color and size then draw the lifebars
    sfSprite_setScale(_sprites[SP_LIFE_BAR], LIFE_BAR_SIZE(_game.player1.life));
    sfSprite_setPosition(_sprites[SP_LIFE_BAR], LIFE_BAR_POS_P1);
    sfRenderWindow_drawSprite(_window, _sprites[SP_LIFE_BAR], NULL);
    sfSprite_setScale(_sprites[SP_LIFE_BAR], LIFE_BAR_SIZE(_game.player2.life));
    sfSprite_setPosition(_sprites[SP_LIFE_BAR], LIFE_BAR_POS_P2);
    sfRenderWindow_drawSprite(_window, _sprites[SP_LIFE_BAR], NULL);
    // set the position, color and size then draw the background of the lifebars
    sfSprite_setPosition(_sprites[SP_LIFE_BAR_BOUNDS], LIFE_BAR_BOUNDS_POS_P1);
    sfRenderWindow_drawSprite(_window, _sprites[SP_LIFE_BAR_BOUNDS], NULL);
    sfSprite_setPosition(_sprites[SP_LIFE_BAR_BOUNDS], LIFE_BAR_BOUNDS_POS_P2);
    sfRenderWindow_drawSprite(_window, _sprites[SP_LIFE_BAR_BOUNDS], NULL);
    
    // when one of the player die show a little message to the screen
    if (_game.player1.life == 0)
    {
        sfText_setString(_text, "JOUEUR 2 GAGNE !");
        sfFloatRect rect = sfText_getGlobalBounds(_text);
        sfText_setOrigin(_text, (sfVector2f){ rect.width / 2, rect.height / 2 });
        sfText_setPosition(_text, (sfVector2f){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 });
        sfRenderWindow_drawText(_window, _text, NULL);
    }
    if (_game.player2.life == 0)
    {
        sfText_setString(_text, "JOUEUR 1 GAGNE !");
        sfFloatRect rect = sfText_getGlobalBounds(_text);
        sfText_setOrigin(_text, (sfVector2f){ rect.width / 2, rect.height / 2 });
        sfText_setPosition(_text, (sfVector2f){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 });
        sfRenderWindow_drawText(_window, _text, NULL);
    }
    
    sfRenderWindow_display(_window);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    srand((unsigned int)time(0));
    
    // creates the window
    sfVideoMode mode = { SCREEN_WIDTH, SCREEN_HEIGHT, BIT_PER_PIXEL };
    sfRenderWindow* window =
            sfRenderWindow_create(mode, "Ramboolean", sfResize | sfClose, NULL);
    sfRenderWindow_setKeyRepeatEnabled(window, FALSE);
    
    // creates the battlefield
    sfImage* fieldImg = sfImage_createFromFile("resources/battle_field.png");
    sfTexture* fieldTexture = sfTexture_createFromImage(fieldImg, NULL);
    
    // creates a tab of sprite to store all the sprites at the same place
    // the we load all the sprites
    sfSprite* sprites[SP_COUNT];
    sprites[SP_BACKGROUND] = LoadSprite("resources/background.png", FALSE);
    sprites[SP_BATTLE_FIELD] = sfSprite_create();
    sfSprite_setTexture(sprites[SP_BATTLE_FIELD], fieldTexture, TRUE);
    sprites[SP_TURRET_P1] = LoadSprite("resources/Player1.png", TRUE);
    sprites[SP_TURRET_P2] = LoadSprite("resources/Player2.png", TRUE);
    sprites[SP_INVENTORY] = LoadSprite("resources/inventaire.png", FALSE);
    sprites[SP_SELECTOR] = LoadSprite("resources/selector.png", TRUE);
    sprites[SP_GRENADE] = LoadSprite("resources/grenade.png", TRUE);
    sprites[SP_ROCKET] = LoadSprite("resources/boulet.png", TRUE);
    sprites[SP_KNIFE] = LoadSprite("resources/knife.png", TRUE);
    sprites[SP_MIRE] = LoadSprite("resources/mire.png", TRUE);
    sprites[SP_LEAF] = LoadSprite("resources/snow_ball.png", TRUE);
    sprites[SP_LIFE_BAR] = LoadSprite("resources/life_bar.png", FALSE);
    sprites[SP_LIFE_BAR_BOUNDS] =
            LoadSprite("resources/life_bar_background.png", FALSE);
    sprites[SP_GAUGE] = LoadSprite("resources/power_gauge.png", FALSE);
    sfSprite_setOrigin(sprites[SP_GAUGE], POWER_GAUGE_ORIGIN);
    
    // ditto with the sounds
    sfSound* sounds[SD_COUNT];
    sounds[SD_HURT] = LoadSound("resources/Wilhelm.ogg");
    
    sfFont* font = sfFont_createFromFile("resources/Gamer.ttf");
    sfText* text = sfText_create();
    sfText_setFont(text, font);
    sfText_setCharacterSize(text, CHARACTER_SIZE);
    sfText_setColor(text, sfBlack);
    
    Game game;
    sfClock* gameClock = sfClock_create();
    game.timePerFrame = sfTime_Zero;
    
    game = Initialize(game, fieldImg, fieldTexture);
    sfClock_restart(gameClock);
    while (sfRenderWindow_isOpen(window))
    {
        game.timePerFrame.microseconds += sfClock_restart(gameClock).microseconds;
        game = Input(game, window, fieldImg, fieldTexture);
        // we do a game loop all DELTA_TIME to make the game independent of
        // the computer power
        if (sfTime_asSeconds(game.timePerFrame) > DELTA_TIME)
        {
            game.timePerFrame.microseconds -= MICROSECONDS(DELTA_TIME);
            game = Update(game, fieldImg, fieldTexture, sprites, sounds);
            Display(game, window, sprites, text);
        }
    }
    
    return 0;
}

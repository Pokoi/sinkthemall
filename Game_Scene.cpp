/*
 * GAME SCENE
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

/*
 * MODIFIED BY
 *
 * Jesus 'Pokoi' Villar
 * © pokoidev 2019 (pokoidev.com)
 *
 * Creative Commons License:
 * Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "Game_Scene.hpp"

#include <cstdlib>
#include <basics/Accelerometer>
#include <basics/Canvas>
#include <basics/Director>

using namespace basics;
using namespace std;

namespace jesus_villar_examen
{
     constexpr float     Game_Scene::bullet_speed              ;     ///< Velocidad a la que se mueve el proyectil (en unideades virtuales por segundo).
     constexpr float     Game_Scene::ship_speed                ;     ///< Velocidad a la que se mueve el barco (en unideades virtuales por segundo).
     constexpr float     Game_Scene::submarine_speed           ;     ///< Velocidad a la que se mueven los submarinos (en unidades virtuales por segundo)
     constexpr unsigned  Game_Scene::number_of_player_bullets  ;        ///< Número de balas
     constexpr unsigned  Game_Scene::number_of_enemy_bullets   ;        ///< Número de balas
     constexpr unsigned  Game_Scene::number_of_submarines      ;        ///< Número de submarinos


    // ---------------------------------------------------------------------------------------------
    // ID y ruta de las texturas que se deben cargar para esta escena.

    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(ship),      "game-scene/boat.png"},
        { ID(submarine),  "game-scene/submarine.png"},
        { ID(bullet),  "game-scene/bullet.png"},
        { ID(water),  "game-scene/water.png"},

        //...

    };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaño en bytes de un item:

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);




    // ---------------------------------------------------------------------------------------------

    Game_Scene::Game_Scene()
    {
        // Se establece la resolución virtual (independiente de la resolución virtual del dispositivo).
        // En este caso no se hace ajuste de aspect ratio, por lo que puede haber distorsión cuando
        // el aspect ratio real de la pantalla del dispositivo es distinto.

        canvas_width  = 1280;
        canvas_height =  720;

        aspect_ratio_adjusted = false;


        // Se inicia la semilla del generador de números aleatorios:
        srand (unsigned(time(nullptr)));

        // Se inicializan otros atributos:

        initialize ();
    }

    // ---------------------------------------------------------------------------------------------
    // Algunos atributos se inicializan en este método en lugar de hacerlo en el constructor porque
    // este método puede ser llamado más veces para restablecer el estado de la escena y el constructor
    // solo se invoca una vez.

    bool Game_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;
        gameplay  = UNINITIALIZED;

        timer.reset();

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::suspend ()
    {
        suspended = true;               // Se marca que la escena ha pasado a primer plano

        Accelerometer * accelerometer = Accelerometer::get_instance ();

        if (accelerometer) accelerometer->switch_off ();
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::resume ()
    {
        suspended = false;              // Se marca que la escena ha pasado a segundo plano

        Accelerometer * accelerometer = Accelerometer::get_instance ();

        if (accelerometer) accelerometer->switch_on ();
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::handle (Event & event)
    {
        if (state == RUNNING)               // Se descartan los eventos cuando la escena está LOADING
        {
            if (gameplay == WAITING_TO_START)
            {
                start_playing ();           // Se empieza a jugar cuando el usuario toca la pantalla
                                            // por primera vez
            }
            else switch (event.id)
            {
                case ID(touch-started):     // El usuario toca la pantalla
                {

                    spawn_bullet();

                    break;
                }
                case ID(touch-moved):
                {

                    break;
                }

                case ID(touch-ended):       // El usuario deja de tocar la pantalla
                {

                    break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update (float time)
    {
        if (!suspended) switch (state)
        {
            case LOADING: load_textures  ();     break;
            case RUNNING: run_simulation (time); break;
            case ERROR:   break;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render (Context & context)
    {
        if (!suspended)
        {
            // El canvas se puede haber creado previamente, en cuyo caso solo hay que pedirlo:

            Canvas * canvas = context->get_renderer< Canvas > (ID(canvas));

            // Si no se ha creado previamente, hay que crearlo una vez:

            if (!canvas)
            {
                 canvas = Canvas::create (ID(canvas), context, {{ canvas_width, canvas_height }});
            }

            // Si el canvas se ha podido obtener o crear, se puede dibujar con él:

            if (canvas)
            {
                canvas->clear ();

                switch (state)
                {
                    case LOADING: render_loading   (*canvas); break;
                    case RUNNING: render_playfield (*canvas); break;
                    case ERROR:   break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------
    // En este método solo se carga una textura por fotograma para poder pausar la carga si el
    // juego pasa a segundo plano inesperadamente. Otro aspecto interesante es que la carga no
    // comienza hasta que la escena se inicia para así tener la posibilidad de mostrar al usuario
    // que la carga está en curso en lugar de tener una pantalla en negro que no responde durante
    // un tiempo.

    void Game_Scene::load_textures ()
    {
        if (textures.size () < textures_count)          // Si quedan texturas por cargar...
        {
            // Las texturas se cargan y se suben al contexto gráfico, por lo que es necesario disponer
            // de uno:

            Graphics_Context::Accessor context = director.lock_graphics_context ();

            if(!aspect_ratio_adjusted){

                adjust_aspect_ratio(context);
            }

            if (context)
            {
                // Se ajusta el aspect ratio si este no se ha ajustado


                // Se carga la siguiente textura (textures.size() indica cuántas llevamos cargadas):

                Texture_Data   & texture_data = textures_data[textures.size ()];
                Texture_Handle & texture      = textures[texture_data.id] = Texture_2D::create (texture_data.id, context, texture_data.path);

                // Se comprueba si la textura se ha podido cargar correctamente:

                if (texture) context->add (texture); else state = ERROR;

                // Cuando se han terminado de cargar todas las texturas se pueden crear los gameobjects que
                // las usarán e iniciar el juego:
            }
        }
        else
        if (timer.get_elapsed_seconds () > 1.f)         // Si las texturas se han cargado muy rápido
        {                                               // se espera un segundo desde el inicio de
            create_gameobjects();                          // la carga antes de pasar al juego para que
            restart_game   ();                          // el mensaje de carga no aparezca y desaparezca
                                                        // demasiado rápido.
            state = RUNNING;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::create_gameobjects()
    {

        //TODO: crear y configurar los gameobjects de la escena
        // Se crean y configuran los gameobjects:

        //GameObject_Handle  nombre_objeto(new GameObject (textures[ID(nombre_ID)].get() ));
        //...

        GameObject_Handle barco     (new GameObject (textures[ID(ship)]   .get() ));
        GameObject_Handle water     (new GameObject (textures[ID(water)]  .get() ));

        //Se establecen los anchor y position de los GameObject

        //nombre_objeto->set_anchor (...);
        //nombre_objeto->set_position ({coordenada_x, coordenada_y});

        barco -> set_anchor(CENTER);
        barco -> set_position({canvas_width * 0.5f, (canvas_height * 0.5f) + (barco -> get_height() * 0.5f)});

        water -> set_anchor(CENTER);
        water -> set_position({canvas_width*0.5f, canvas_height*0.5f});


        //Se añaden a la lista de game objects
        //gameobjects.push_back (nombre_objeto);

        gameobjects.push_back (barco);

        // Se guardan punteros a los gameobjects que se van a usar frecuentemente:

        // nombre_puntero = nombre_objeto.get();

        player_ship_pointer = barco.get();


        // Se crean los proyectiles del jugador
        for(unsigned iterator = 0; iterator < number_of_player_bullets; iterator++)
        {
            GameObject_Handle bullet (new GameObject (textures[ID(bullet)]. get() ));

            bullet -> hide ();

            player_bullets.    push_back(bullet);
            gameobjects.push_back(bullet);
        }

        // Se crean los proyectiles del enemigo
        for(unsigned iterator = 0; iterator < number_of_enemy_bullets; iterator++)
        {
            GameObject_Handle bullet (new GameObject (textures[ID(bullet)]. get() ));

            bullet -> hide ();

            enemy_bullets.    push_back(bullet);
            gameobjects.push_back(bullet);
        }

        // Se crean los submarinos
        for (unsigned iterator = 0; iterator < number_of_submarines; iterator++)
        {

            GameObject_Handle submarine(new GameObject(textures[ID(submarine)].get()));

            random_submarine_values(*submarine);

            submarines.push_back(submarine);
            gameobjects.push_back(submarine);

        }

    }

    // ---------------------------------------------------------------------------------------------
    // Cuando el juego se inicia por primera vez o cuando se reinicia porque un jugador pierde, se
    // llama a este método para restablecer los gameobjects:

    void Game_Scene::restart_game()
    {
        // TODO: resetear valores iniciales de los gameobjects que lo requieran

        player_ship_pointer -> set_position({canvas_width * 0.5f, (canvas_height * 0.5f) + (player_ship_pointer -> get_height() * 0.5f)});
        player_ship_pointer -> set_speed({0,0});

        for (auto & gameobject : enemy_bullets)
        {
           gameobject -> hide();
           gameobject -> set_speed_y(0);

        }

        for (auto & gameobject : player_bullets)
        {
            gameobject -> hide();
            gameobject -> set_speed_y(0);

        }

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::start_playing ()
    {
        //TODO: Implementar las cosas que se tengan que realizar al empezar a jugar

        gameplay = PLAYING;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::run_simulation (float time)
    {

        // Calculamos la velocidad del barco en función del acelerómetro
        ship_movement();

        // Evitamos que el barco salga de los límites
        fix_ship_position();

        // Nos subscribimos al update de todos los objetos
        for (auto & gameobject : gameobjects)
        {
            gameobject->update (time);
        }

        // Comprobamos si las balas del jugador se salen de rango
        // o si chocan con un submarino

        for (auto & gameobject : player_bullets)
        {
            if(gameobject -> is_visible())
            {
                if(gameobject -> get_top_y() <= 0)
                {
                    gameobject -> hide();
                    gameobject -> set_speed_y(0);
                }

                for (auto & submarine : submarines)
                {
                    if (gameobject -> intersects(*submarine))
                    {
                        gameobject -> hide();
                        gameobject -> set_speed_y(0);

                        random_submarine_values(*submarine);
                    }
                }
            }
        }

        // Comprobamos si las balas del enemigo se salen de rango
        // o si chocan con el jugador
        for (auto & gameobject : enemy_bullets)
        {
            if (gameobject -> is_visible())
            {
                if(gameobject -> get_top_y() >= canvas_height * 0.5f)
                {
                    gameobject -> hide();
                    gameobject -> set_speed_y(0);

                    if(gameobject -> intersects(*player_ship_pointer))
                    {
                        player_ship_pointer -> set_speed_y(-300);

                    }
                }

            }
        }


        // Comprobamos si los submarinos salen de la pantalla
        for (auto & submarine : submarines)
        {
            if(submarine -> get_speed_x() > 0 && submarine -> get_left_x() >= canvas_width){
                random_submarine_values( *submarine);
            }

        }

        // Dispara el enemigo
        if(timer.get_elapsed_seconds () > 2.f)
        {
            spawn_enemy_bullet();
            timer.reset();
        }


        //TODO: implementación de la IA

        //TODO: implementación de posibles colisiones
    }


    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render_loading (Canvas & canvas)
    {
        //TODO: tiene que haber alguna textura con ID loading para la carga
        /*Texture_2D * loading_texture = textures[ID(loading)].get ();

        if (loading_texture)
        {
            canvas.fill_rectangle
            (
                { canvas_width * .5f, canvas_height * .5f },
                { loading_texture->get_width (), loading_texture->get_height () },
                  loading_texture
            );
        }*/
    }

    // ---------------------------------------------------------------------------------------------
    // Simplemente se dibujan todos los gameobjects que conforman la escena.

    void Game_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & gameobject : gameobjects)
        {
            gameobject->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Ajusta el aspect ratio

    void Game_Scene::adjust_aspect_ratio(Context & context)
    {



        float real_aspect_ratio = float( context->get_surface_width () ) / context->get_surface_height ();

        canvas_width = unsigned ( canvas_height * real_aspect_ratio);

        aspect_ratio_adjusted = true;
    }

    // ---------------------------------------------------------------------------------------------
    // Ajusta la velocidad del barco

    void Game_Scene::ship_movement() {

        Accelerometer * accelerometer = Accelerometer::get_instance ();

        if (accelerometer) {
            const Accelerometer::State &acceleration = accelerometer->get_state();

            float pitch = atan2f(-acceleration.x, sqrtf(acceleration.y * acceleration.y +
                                                        acceleration.z * acceleration.z)) * ship_speed;

            player_ship_pointer->set_speed_x(pitch);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Ajusta la posición del barco

    void Game_Scene::fix_ship_position(){

        if( player_ship_pointer -> get_right_x() >= canvas_width)
        {
            player_ship_pointer -> set_position_x(canvas_width - player_ship_pointer -> get_width() * 0.5f);

        }
        else if( player_ship_pointer -> get_left_x() <= 0)
        {
            player_ship_pointer -> set_position_x(player_ship_pointer -> get_width() * 0.5f);
        }

        if( player_ship_pointer -> get_top_y() <= 0){
            restart_game();
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Spawnea una bala del jugador

    void Game_Scene::spawn_bullet ()
    {
        unsigned iterator;
        for(iterator = 0; iterator < player_bullets.size() && player_bullets[iterator] -> is_visible(); iterator++)
        {};

        if(iterator < player_bullets.size())
        {

            player_bullets[iterator] -> set_position({player_ship_pointer -> get_position_x(), (player_ship_pointer -> get_position_y()) - (player_ship_pointer -> get_height() * 0.5f)});
            player_bullets[iterator] -> set_speed({0, -bullet_speed});
            player_bullets[iterator] -> show();
        }

    }

    // ---------------------------------------------------------------------------------------------
    // Proporciona valores aleatorios a los submarinos

    void Game_Scene::random_submarine_values(GameObject & submarine){

        float speed = submarine_speed + (-100 + float((rand () % int (100))));
        float y  = submarine.get_height()*0.5f + float(rand () % int((canvas_height * 0.5f) - submarine.get_height()));
        submarine.set_position({-20,y});
        submarine.set_speed_x(speed);

    }

    // ---------------------------------------------------------------------------------------------
    // Genera las balas del enemigo

    void Game_Scene::spawn_enemy_bullet()
    {
        int position = rand () % int (submarines.size());

        unsigned iterator;
        for(iterator = 0; iterator < enemy_bullets.size() && enemy_bullets[iterator] -> is_visible(); iterator++)
        {};

        if(iterator < enemy_bullets.size())
        {

            enemy_bullets[iterator] -> set_position({submarines[iterator] -> get_position_x(), (submarines [iterator] -> get_position_y()) + (submarines [iterator] -> get_height() * 0.5f)});
            enemy_bullets[iterator] -> set_speed({0, bullet_speed});
            enemy_bullets[iterator] -> show();
        }

    }

}

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

#ifndef GAME_SCENE_HEADER
#define GAME_SCENE_HEADER

    #include <map>
    #include <list>
    #include <memory>

    #include <basics/Canvas>
    #include <basics/Id>
    #include <basics/Scene>
    #include <basics/Texture_2D>
    #include <basics/Timer>

    #include "GameObject.hpp"

    namespace jesus_villar_examen
    {

        using basics::Id;
        using basics::Timer;
        using basics::Canvas;
        using basics::Texture_2D;

        class Game_Scene : public basics::Scene
        {

            // Estos typedefs pueden ayudar a hacer el código más compacto y claro:

            typedef std::shared_ptr < GameObject >         GameObject_Handle;
            typedef std::vector< GameObject_Handle >         GameObject_List;
            typedef std::shared_ptr< Texture_2D  >         Texture_Handle;
            typedef std::map< Id, Texture_Handle >         Texture_Map;
            typedef basics::Graphics_Context::Accessor     Context;

            /**
             * Representa el estado de la escena en su conjunto.
             */
            enum State
            {
                LOADING,
                RUNNING,
                ERROR
            };

            /**
             * Representa el estado del juego cuando el estado de la escena es RUNNING.
             */
            enum Gameplay_State
            {
                UNINITIALIZED,
                WAITING_TO_START,
                PLAYING,
                ENDING,
            };

        private:

            /**
             * Array de estructuras con la información de las texturas (Id y ruta) que hay que cargar.
             */
            static struct   Texture_Data { Id id; const char * path; } textures_data[];

            /**
             * Número de items que hay en el array textures_data.
             */
            static unsigned textures_count;

        private:

            static constexpr float    bullet_speed              = 400.f;     ///< Velocidad a la que se mueve el proyectil (en unideades virtuales por segundo).
            static constexpr float    ship_speed                = 600.f;     ///< Velocidad a la que se mueve el barco (en unideades virtuales por segundo).
            static constexpr float    submarine_speed           = 200.f;     ///< Velocidad a la que se mueven los submarinos (en unidades virtuales por segundo)
            static constexpr unsigned number_of_player_bullets  = 50;        ///< Número de balas
            static constexpr unsigned number_of_enemy_bullets   = 10;        ///< Número de balas
            static constexpr unsigned number_of_submarines      = 4;        ///< Número de submarinos


        private:

            State          state;                               ///< Estado de la escena.
            Gameplay_State gameplay;                            ///< Estado del juego cuando la escena está RUNNING.
            bool           suspended;                           ///< true cuando la escena está en segundo plano y viceversa.

            unsigned       canvas_width;                        ///< Ancho de la resolución virtual usada para dibujar.
            unsigned       canvas_height;                       ///< Alto  de la resolución virtual usada para dibujar.
            bool           aspect_ratio_adjusted;               ///< False hasta que se ajuste el aspect ratio de la resolución.

            Texture_Map        textures;                        ///< Mapa  en el que se guardan shared_ptr a las texturas cargadas.
            GameObject_List    gameobjects;                     ///< Lista en la que se guardan shared_ptr a los gameobject creados.
            GameObject_List    player_bullets;                  ///< Lista de balas del jugador
            GameObject_List    enemy_bullets;                   ///< Lista de balas de los submarinos
            GameObject_List    submarines;                      ///< Lista de submarinos

            GameObject       * left_border;                     ///< Puntero al game object de la lista de game objects que representa el borde izquierdo.
            GameObject       * bottom_border;                   ///< Puntero al game object de la lista de game objects que representa el borde inferior.
            GameObject       * right_border;                    ///< Puntero al game object de la lista de game objects que representa el borde derecho.

            GameObject       * player_ship_pointer;             ///< Puntero al game object de la lista de game objects que representa el barco del jugador.


            Timer          timer;                               ///< Cronómetro usado para medir intervalos de tiempo

        public:

            /**
             * Solo inicializa los atributos que deben estar inicializados la primera vez, cuando se
             * crea la escena desde cero.
             */
            Game_Scene();

            /**
             * Este método lo llama Director para conocer la resolución virtual con la que está
             * trabajando la escena.
             * @return Tamaño en coordenadas virtuales que está usando la escena.
             */
            basics::Size2u get_view_size () override
            {
                return { canvas_width, canvas_height };
            }

            /**
             * Aquí se inicializan los atributos que deben restablecerse cada vez que se inicia la escena.
             * @return
             */
            bool initialize () override;

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a segundo plano.
             */
            void suspend () override;

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a primer plano.
             */
            void resume () override;

            /**
             * Este método se invoca automáticamente una vez por fotograma cuando se acumulan
             * eventos dirigidos a la escena.
             */
            void handle (basics::Event & event) override;

            /**
             * Este método se invoca automáticamente una vez por fotograma para que la escena
             * actualize su estado.
             */
            void update (float time) override;

            /**
             * Este método se invoca automáticamente una vez por fotograma para que la escena
             * dibuje su contenido.
             */
            void render (Context & context) override;

        private:

            /**
             * En este método se cargan las texturas (una cada fotograma para facilitar que la
             * propia carga se pueda pausar cuando la aplicación pasa a segundo plano).
             */
            void load_textures ();

            /**
             * En este método se crean los gameobjects cuando termina la carga de texturas.
             */
            void create_gameobjects();

            /**
             * Se llama cada vez que se debe reiniciar el juego. En concreto la primera vez y cada
             * vez que un jugador pierde.
             */
            void restart_game ();

            /**
             * Cuando se ha reiniciado el juego y el usuario toca la pantalla por primera vez se
             * pone la bola en movimiento en una dirección al azar.
             */
            void start_playing ();

            /**
             * Actualiza el estado del juego cuando el estado de la escena es RUNNING.
             */
            void run_simulation (float time);


            /**
             * Dibuja la textura con el mensaje de carga mientras el estado de la escena es LOADING.
             * La textura con el mensaje se carga la primera para mostrar el mensaje cuanto antes.
             * @param canvas Referencia al Canvas con el que dibujar la textura.
             */
            void render_loading (Canvas & canvas);

            /**
             * Dibuja la escena de juego cuando el estado de la escena es RUNNING.
             * @param canvas Referencia al Canvas con el que dibujar.
             */
            void render_playfield (Canvas & canvas);

            /**
             * Ajusta el aspect ratio
             */
            void adjust_aspect_ratio(Context & context);

            /**
             * Se mueve al barco en función del acelerómetro
             */
            void ship_movement();

            /**
             * Método que controla que el barco no se salga de la pantalla
             */
            void fix_ship_position();

            /**
             * Método que genera una bala del jugador en la escena
             */
            void spawn_bullet();

            /**
             * Método que genera una bala de los enemigos en la escena
             */
            void spawn_enemy_bullet();

            /**
             * Método que da unos valores random a los submarinos
             */
            void random_submarine_values(GameObject & submarine);



        };

    }

#endif

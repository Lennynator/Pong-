#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <memory>
/// K‰ytet‰‰n smart pointereita valvomaan ruudun ja SDL:n k‰yttˆ‰

/// <summary>
/// ruudun leveys
/// </summary>
const int SCREEN_WIDTH = 640;
/// <summary>
/// ruudun pituus
/// </summary>
const int SCREEN_HEIGHT = 480;

/// Paddlejen tiedot
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 100;
const int PADDLE_OFFSET = 50;
const int PADDLE_SPEED = 10;

/// Pallon tiedot
const int BALL_SIZE = 10;
const float BALL_SPEED_X = 5.0f; 
const float BALL_SPEED_Y = 5.0f; 

/// <summary>
/// Poistetaan SDL_Window kun sit‰ ei en‰‰ tarvita
/// </summary>
struct SDLWindowDeleter {
    void operator()(SDL_Window* window) const {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
};


/// <summary>
/// Poistetaan SDL_Renderer kun sit‰ ei en‰‰ tarvita
/// </summary>
struct SDLRendererDeleter {
    void operator()(SDL_Renderer* renderer) const {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
    }
};


/// <summary>
/// Parsataan tiedot inputeista, sen sijaan ett‰ k‰sitelt‰isiin ne p‰‰ohjelmassa
/// </summary>
/// <param name="state">onko tietyj‰ n‰pp‰imi‰ painettu</param>
/// <param name="leftPaddle">Vasen padeli</param>
/// <param name="rightPaddle">Oikea padeli</param>
void parseInput(const Uint8* state, SDL_Rect& leftPaddle, SDL_Rect& rightPaddle) {
    if (state[SDL_SCANCODE_W]) {
        leftPaddle.y -= PADDLE_SPEED;
        if (leftPaddle.y < 0) {
            leftPaddle.y = 0;
        }
    }

    if (state[SDL_SCANCODE_S]) {
        leftPaddle.y += PADDLE_SPEED;
        if (leftPaddle.y > (SCREEN_HEIGHT - PADDLE_HEIGHT)) {
            leftPaddle.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
        }
    }

    if (state[SDL_SCANCODE_UP]) {
        rightPaddle.y -= PADDLE_SPEED;
        if (rightPaddle.y < 0) {
            rightPaddle.y = 0;
        }
    }

    if (state[SDL_SCANCODE_DOWN]) {
        rightPaddle.y += PADDLE_SPEED;
        if (rightPaddle.y > (SCREEN_HEIGHT - PADDLE_HEIGHT)) {
            rightPaddle.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
        }
    }
}

/// <summary>
/// Funktio pallon liikutusta varten
/// </summary>
/// <param name="ball">Pallo itsess‰‰n</param>
/// <param name="ballVelX">Pallon X suunntainen vektori</param>
/// <param name="ballVelY">Pallon Y suuntainen vektori</param>
/// <param name="leftPaddle"> Vasen padeli</param>
/// <param name="rightPaddle"> Oikea padeli</param>
void moveBall(SDL_Rect& ball, float& ballVelX, float& ballVelY, const SDL_Rect& leftPaddle, const SDL_Rect& rightPaddle) {
    ball.x += ballVelX;
    ball.y += ballVelY;

    if (ball.y <= 0 || ball.y + BALL_SIZE >= SCREEN_HEIGHT) {
        ballVelY = -ballVelY; 
    }

    if (ball.x <= leftPaddle.x + PADDLE_WIDTH &&
        ball.x >= leftPaddle.x &&
        ball.y + BALL_SIZE >= leftPaddle.y &&
        ball.y <= leftPaddle.y + PADDLE_HEIGHT) {
        ballVelX = -ballVelX;  
    }

    if (ball.x + BALL_SIZE >= rightPaddle.x &&
        ball.x <= rightPaddle.x + PADDLE_WIDTH &&
        ball.y + BALL_SIZE >= rightPaddle.y &&
        ball.y <= rightPaddle.y + PADDLE_HEIGHT) {
        ballVelX = -ballVelX;  
    }

    if (ball.x <= 0 || ball.x + BALL_SIZE >= SCREEN_WIDTH) {
        ball.x = (SCREEN_WIDTH - BALL_SIZE) / 2;
        ball.y = (SCREEN_HEIGHT - BALL_SIZE) / 2;
        ballVelX = (ballVelX > 0) ? BALL_SPEED_X : -BALL_SPEED_X; 
    }
}

/// <summary>
/// P‰‰ohjelma, jossa loopilla kutsutaan parsetettuja liikkumiskutsuja kunnes pelaaja lopettaa pelin
/// </summary>
/// <param name="argc">Command line argumentien m‰‰r‰</param>
/// <param name="argv">Comman line arrayden m‰‰r‰</param>
/// <returns>Palauttaa 0 kun peli loppuu klikkaamalla pois pelist‰s</returns>
int main(int argc, char* argv[]) {
    std::unique_ptr<SDL_Window, SDLWindowDeleter> window(nullptr);
    std::unique_ptr<SDL_Renderer, SDLRendererDeleter> renderer(nullptr);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    window.reset(SDL_CreateWindow("Pong Paddles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN));
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(renderer.get());

    SDL_Rect leftPaddle = { PADDLE_OFFSET, (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2, PADDLE_WIDTH, PADDLE_HEIGHT };

    SDL_Rect rightPaddle = { SCREEN_WIDTH - PADDLE_OFFSET - PADDLE_WIDTH, (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2, PADDLE_WIDTH, PADDLE_HEIGHT };

    SDL_Rect ball = { (SCREEN_WIDTH - BALL_SIZE) / 2, (SCREEN_HEIGHT - BALL_SIZE) / 2, BALL_SIZE, BALL_SIZE };
    float ballVelX = BALL_SPEED_X; 
    float ballVelY = BALL_SPEED_Y; 

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);

        parseInput(state, leftPaddle, rightPaddle);

        moveBall(ball, ballVelX, ballVelY, leftPaddle, rightPaddle);

        SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer.get());

        SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
        SDL_RenderFillRect(renderer.get(), &leftPaddle);
        SDL_RenderFillRect(renderer.get(), &rightPaddle);

        SDL_RenderFillRect(renderer.get(), &ball);

        SDL_RenderPresent(renderer.get());

        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}





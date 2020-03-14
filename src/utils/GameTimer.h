// TODO: planning on implementing a game timer with changeable time scale 
// to allow for pausing as well as time warping (speed up/slow down) effects
class GameTimer
{
private:
    float previousTime;
    float currentTime;
    float timeScale; 
public:
    GameTimer();
    ~GameTimer();
};

GameTimer::GameTimer()
{
    timeScale = 1;
}

GameTimer::~GameTimer()
{
}

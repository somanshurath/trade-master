#include "./app/AppLayer.h"

int main(int, char**) {
    AppLayer app;
    
    if (!app.OnStart())
        return -1;

    app.Run();
    app.OnExit();

    return 0;
}

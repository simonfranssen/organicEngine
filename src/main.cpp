#include "../Engine/Core/Application.h"

int main()
{
    Application application;

    if (!application.Init())
        return 1;

    application.Run();

    return 0;
}
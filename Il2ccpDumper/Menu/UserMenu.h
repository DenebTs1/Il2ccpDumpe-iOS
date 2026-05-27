#pragma once

#include "../MenuLoad/Includes.h"


class UserMenu : public Singleton<UserMenu> 
{
public:

    void RenderMenu();
    void SetVisible(bool visible);
    bool IsVisible() const;


private:
    friend class Singleton<UserMenu>;
    UserMenu() {};
    ~UserMenu() {};
};

#pragma once
#include "Source/Slate/Core/Windows/SWindow.h"

class UPropertyWindow;
class SDetailsWindow :
    public SWindow
{
public:
    SDetailsWindow();
    virtual ~SDetailsWindow();

    void Initialize();
    virtual void OnRender() override;
    virtual void OnUpdate(float deltaSecond) override;

private:
    UPropertyWindow* DetailsWidget;
};


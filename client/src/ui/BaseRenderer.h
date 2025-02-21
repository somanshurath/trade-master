#pragma once

#include <string>

class BaseRenderer
{
protected:
    bool m_show_window = true;

public:
    virtual ~BaseRenderer() = default;
    virtual void Render() = 0;

    virtual void FetchData() = 0;

    bool IsVisible() const { return m_show_window; }
    void Show() { m_show_window = true; }
    void Hide() { m_show_window = false; }
    void Toggle() { m_show_window = !m_show_window; }

    virtual std::string tabName() = 0;
};

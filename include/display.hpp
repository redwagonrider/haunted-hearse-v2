#pragma once

namespace Display {
    void init();
    void setStatus(const char* status);
    void setScene(const char* scene);
    bool isAvailable();
}
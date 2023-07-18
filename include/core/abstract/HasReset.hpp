#pragma once

struct HasReset {
    virtual void reset() = 0;
    virtual ~HasReset() = default;
}; // non randoms are resetable

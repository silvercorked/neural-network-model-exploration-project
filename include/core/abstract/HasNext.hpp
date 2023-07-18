#pragma once

struct HasNext {
    virtual bool next() = 0;
    virtual ~HasNext() = default;
};

#ifndef MATHLAB_INPUTUTILS_H
#define MATHLAB_INPUTUTILS_H
#include <istream>
#include <limits>

namespace InputUtils {
    // Returns false ONLY at end of stream — the caller must stop looping.
    // On a recoverable parse error the stream is reset, the offending line is
    // discarded, value is set to -1, and true is returned so the caller can
    // re-prompt. Checking eof() before clear() matters: clearing and ignoring
    // unconditionally would re-set eofbit right away at EOF, so the next
    // extraction would fail again and the menu would spin forever.
    inline bool readInt(std::istream& in, int& value) {
        if (in >> value) return true;
        if (in.eof()) return false;
        in.clear();
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        value = -1;
        return true;
    }
}
#endif

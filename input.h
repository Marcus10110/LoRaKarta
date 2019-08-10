#pragma once

/*
Summary:

when a key is pressed, it's in an editable state.
If nothing happens for a bit, it gets "locked" in.
if another key is pressed, it gets "locked" in.
if the same key is pressed again, then it advances to the next character.
1.5 second timeout.
special charaters:
space -> 0
delete -> 1
done -> X
cancel - #
we can probably maintain the string in-progress here, and have some high level state control.
also, holding a key could trigger a secondary input.
*/

namespace Input
{
    void Scan();
}
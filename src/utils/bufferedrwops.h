/*
 * Buffered SDL_RWops implementation
 * Copyright (C) 2025  Thorbjørn Lindeijer
 *
 * Written as part of a conversation between a human and an AI assistant
 * on GitHub Copilot Chat.
 *
 * This code is released into the public domain. You may use it freely
 * for any purpose, including commercial applications.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

SDL_RWops* createBufferedRWops(SDL_RWops* source);

#ifdef __cplusplus
}
#endif

#pragma once

void
GL_ErrorClear(const char *label);

void
GL_ErrorCheck(const char* fmt, ...);

#define ERROR_CHECK GL_ErrorCheck("%s:%d %s", __FILE__, __LINE__, __func__);

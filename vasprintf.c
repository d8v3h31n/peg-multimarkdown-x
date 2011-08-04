
int vasprintf(char **ret, const char *format, va_list args)
{
        va_list copy;
        va_copy(copy, args);

        /* Make sure it is determinate, despite manuals indicating otherwise */
        *ret = 0;

        int count = vsnprintf(NULL, 0, format, args);
        if (count >= 0) {
                char* buffer = malloc(count + 1);
                if (buffer != NULL) {
                        count = vsnprintf(buffer, count + 1, format, copy);
                        if (count < 0)
                                free(buffer);
                        else
                                *ret = buffer;
                }
        }
        va_end(args);  // Each va_start() or va_copy() needs a va_end()

        return count;
}

int asprintf(char **strp, const char *fmt, ...)
{
        s32 size;
        va_list args;
        va_start(args, fmt);
        size = vasprintf(strp, fmt, args);
        va_end(args);
        return size;
}

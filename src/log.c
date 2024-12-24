#include "log.h"
#include "contact.h"
#include "table.h"
#include "uint256.h"
#include <stdarg.h>

void kad_printf(const char *fmt, ...)
{
    va_list arg_list;
    va_start(arg_list, fmt);
    while (*fmt != '\0')
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
            case 's': // char *.
            {
                char *sval = va_arg(arg_list, char *);
                printf("%s", sval);
                break;
            }

            case 'd': // int.
            {
                int ival = va_arg(arg_list, int);
                printf("%d", ival);
                break;
            }

            case 'U': // kad_uint256_t *.
            {
                kad_uint256_t *uval = va_arg(arg_list, kad_uint256_t *);
                printf("0x");
                for (int i = 0; i < sizeof(uval->d) / sizeof(uval->d[0]); i++)
                {
                    printf("%08x", uval->d[i]);
                }

                break;
            }

            case 'C': // kad_contact_t *.
            {
                kad_contact_t *cval = va_arg(arg_list, kad_contact_t *);
                kad_printf("Contact\n");
                kad_printf("  id:   %U\n", &cval->id);
                kad_printf("  host: %s\n", cval->host);
                kad_printf("  port: %s", cval->port);
                break;
            }

            case 'B': // kad_bucket_t *.
            {
                kad_bucket_t *bval = va_arg(arg_list, kad_bucket_t *);
                kad_ordereddictnode_t *curr;

                kad_printf("Contacts\n");
                curr = bval->contacts.head;
                while (curr)
                {
                    kad_printf("  id: %U\n", &curr->c);
                    curr = curr->next;
                }

                kad_printf("Replacements:\n");
                curr = bval->replacements.head;
                while (curr)
                {
                    kad_printf("  id: %U\n", &curr->c);
                    curr = curr->next;
                }

                break;
            }

            case 'T': // kad_table_t *.
            {
                kad_table_t *tval = va_arg(arg_list, kad_table_t *);
                kad_printf("Table\n");
                kad_printf("  id:       %U\n", &tval->id);
                kad_printf("  capacity: %d\n", tval->capacity);
                kad_printf("  nbuckets: %d\n", tval->nbuckets);
                for (int i = 0; i < tval->nbuckets; i++)
                {
                    kad_printf("---------\n");
                    kad_printf("Bucket %d\n", i);
                    kad_printf("---------\n");
                    kad_printf("%B", &tval->buckets[i]);
                }

                break;
            }
            }

            fmt++;
        }
        else
        {
            printf("%c", *fmt);
            fmt++;
        }
    }

    va_end(arg_list);
}

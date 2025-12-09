# Single Header Libraries

This is a collection of single header libraries.

### Dependencies

Some of the headers depend on other headers. For example all headers depend on `sh_base.h`.
The graph below helps you to figure out which headers you need. Find the header you want to
use and follow the branch from bottom to top to know which headers to include.

If you don't care or just want all you can also just include `sh.h`.

```
                                    sh_base.h
                                        │
       ╭─────────────────────┬──────────┴──────────┬─────────────────────╮
       │                     │                     │                     │
   sh_hash.sh        sh_string_builder.h       sh_base64.h         sh_platform.h
       │                     │                     │
       ╰─────────────────────┼─────────────────────╯
                             │
                      sh_http_server.h
```

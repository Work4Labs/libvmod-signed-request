vmod_signed_request
===================

Varnish parse & decode FB's signed_request module
-------------------------------------------------

Version: 0.1.0

SYNOPSIS
========

import signed_request;


DESCRIPTION
===========

FUNCTIONS
=========

get_page_id
-----------

Prototype
        ::

                get_page_id()

Parameter

Return value
        ::

                string


Description
    get the current page_id from Facebook's signed request.


Example
        ::

                import signed_request;

                vcl_recv{
                  signed_request.get_page_id();
                }

get_is_admin
------------

Prototype
        ::

                get_is_admin()

Parameter

Return value
        ::

                string ("true" or "false")


Description
    get the current page.is_admin status from Facebook's signed request.


Example
        ::

                import signed_request;

                vcl_recv{
                  signed_request.get_is_admin();
                }

get_is_liked
------------

Prototype
        ::

                get_is_liked()

Parameter

Return value
        ::

                string ("true" or "false")


Description
    get the current page.is_liked status from Facebook's signed request.


Example
        ::

                import signed_request;

                vcl_recv{
                  signed_request.get_is_liked();
                }

get_locale
----------

Prototype
        ::

                get_locale()

Parameter

Return value
        ::

                string


Description
    get the current locale from Facebook's signed request.


Example
        ::

                import signed_request;

                vcl_recv{
                  signed_request.get_locale();
                }


INSTALLATION
============

Installation requires a Varnish source tree.

Usage::

    ./autogen.sh
    ./configure VARNISHSRC=DIR [VMODDIR=DIR]

`VARNISHSRC` is the directory of the Varnish source tree for which to
compile your vmod. Both the `VARNISHSRC` and `VARNISHSRC/include`
will be added to the include search paths for your module.

Optionally you can also set the vmod install directory by adding
`VMODDIR=DIR` (defaults to the pkg-config discovered directory from your
Varnish installation).

Make targets:

  * make - builds the vmod
  * make install - installs your vmod in `VMODDIR`
  * make check - runs the unit tests in ``src/tests/*.vtc``

HISTORY
=======

Version 0.1.0: Basic decoding of page_id and locale

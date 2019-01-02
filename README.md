# Win32Common

## What is it?
This repo is for the Win32Common shared library project, which I use as a common code base when experimenting and developing with the Win32 API. It may or may not become part of the Onyx32 framework at some point in the future.

## So it's only for Windows?
Yes, Windows. I do, however, have an interest in developing a Linux variant. My knowledge of Linux C++ development is currently limited, though, and such an undertaking might end up in a separate repo. I can definitely see common ground, however, as stuff like dynamic linking is not unique to Windows.

## How can I use it?
It's largely intended to assist me with playing around and learning Win32 by giving me some common Win32 code, as well as keeping my C++ skills up to date. As such, I don't currently recommend using it as anything other than an educational resource (you'd be far better served with something like Boost, Asio, or POCO C++). However, if you do want to use it, the project currently provides code for converting narrow/wide strings and wrapping DLLs. Much more is planned to come, and the changes will be done in coordination with my Onyx32 repo, which is itself due for a major overhaul I've been considering for months.

## What's Onyx32?
It's my prototype GUI framework for making Win32 apps. It's mainly worked on when I get overwhelmed learning about Python/Docker/Linux/Flask/Django. Some existing functionality in Onyx32 is due to be ported to Win32Common.

## When will you finish it?
In due course depending on my other projects. I typically chase multiple projects and ideas and the limited activity you see on GitHub is only a fraction of what I do. :-)
# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Fraxinus** is the public/open-source component of a bronchoscopy navigation application built on top of **CustusX**, a research platform for Image-Guided Surgery and Navigation maintained by SINTEF Medical Technology. Fraxinus itself has a public part (this repo) and a closed-source private extension (`org.custusx.fraxinus.private`, cloned inside this repo when present).

Every CustusX-based app's build/install script (including this repo's `script/cxFraxinusInstaller.py`, via `cxCustusXFinder.py`) auto-checks-out CustusX as a sibling under the same parent directory the first time it runs, so `CX` is reliably present after a build — you don't need to build Fraxinus starting from a CustusX checkout. Its exact folder name depends on how this Fraxinus checkout itself is named: sibling `CX/CX` when this repo's folder is named `FX` (the common case), or sibling `custusx/CustusX` when this repo's folder is named `Fraxinus`. CustusX's own `CLAUDE.md` (`<sibling>/.claude/CLAUDE.md`) documents the full multi-repo build/architecture picture in more depth — check there if it exists; this file only covers what's specific to working in this repo.

## Git remote gets rewritten by the open-source build/install script

`cx.build.cxInstallData.Common` (in the CustusX install-tooling package this repo's `script/cxsetup/cxPublicComponentAssembly.py` builds on) defaults `git_use_https = True`, and the build/install script calls `gitSetRemoteURL()` on every component during `update()` — including this repo itself. Running Fraxinus's public build/install script (`script/cxFraxinusInstaller.py`) therefore resets this repo's `origin` remote to:

```
https://gitlab.sintef.no/custusx/fraxinus.git
```

even if it was previously an SSH URL. This is intentional — Fraxinus's public repo is open source and https doesn't require an SSH key to clone — but it means **`git push` will fail with an HTTP Basic auth error** any time after the build script has run (read/fetch still works fine over https, only push needs SSH auth). CustusX's own installer does the same thing to `CX/CX`'s remote, if you're working there too.

To push, switch to SSH first, push, then switch back so the next build-script run doesn't fight with your remote:

```bash
git remote set-url origin git@gitlab.sintef.no:custusx/Fraxinus.git
git push origin <branch>
git remote set-url origin https://gitlab.sintef.no/custusx/fraxinus.git
```

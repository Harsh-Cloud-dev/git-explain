# git-explain

`git-explain` is a lightweight CLI tool written in C++ that explains Git errors in simple, human-friendly language.

Instead of squinting at a cryptic `fatal:` message and googling it, just run your git command through `git-explain`:

```bash
git-explain push origin main
```

If it succeeds, you get a clean confirmation. If it fails, you get a plain-English diagnosis of what went wrong and a numbered list of commands to fix it — with anything destructive (like `--force`) clearly flagged.

```
✗ git command failed — analyzing error…

── diagnosis ───────────────────────────────────────────────

  Error type : Push Rejected
  Root cause : Your local branch is behind the remote branch.

── what happened ───────────────────────────────────────────

  Someone else pushed commits to the remote since you last pulled. Git
  will not overwrite those commits with a plain push. Pull the remote
  changes first, resolve any conflicts, then push again.

── how to fix it ───────────────────────────────────────────

  1. Pull the latest changes and rebase your work on top
     $ git pull --rebase

  2. Then push again
     $ git push

  3. Force-push if you own the branch and want to overwrite remote (DANGEROUS)
  ⚠  $ git push --force-with-lease
```

## Features

- **Plain-English diagnosis** for ~25 common git error types: merge conflicts, rejected pushes, detached HEAD, auth failures, unknown commands, missing tracking branches, and more.
- **Step-by-step fix commands**, with dangerous operations (force-push, `git clean -f`, branch deletion) clearly marked with ⚠.
- **Full interactive command support.** Commands that need a real editor or terminal — `commit` with no `-m`, `rebase -i`, `add -p`, `tag -a` with no `-m` — are handed directly to git with zero interception, so they behave exactly like running git yourself.
- **Live output streaming** for everything else, including git's own colored output (via a pseudo-terminal), so it looks and feels like running git normally.
- **Run history.** `git-explain --history` shows your recent commands, their status, and what went wrong, persisted to `~/.git-explain/history.log`.
- **Ctrl+C / signal handling** — interrupting a running command works as expected.

## Requirements

- A C++17 compiler (`g++` or `clang++`)
- `make`
- POSIX (Linux or macOS). Windows is not fully supported — see [Platform notes](#platform-notes).

## Building

```bash
make          # builds the git-explain binary
make test     # builds and runs the unit test suite
make all      # builds both
make clean    # removes built binaries
```

## Usage

Prefix any git command with `git-explain`:

```bash
git-explain status
git-explain push origin main
git-explain merge feature-branch
git-explain commit -m "message"
```

Other flags:

```bash
git-explain --help       # usage info
git-explain --version    # version info
git-explain --history    # recent run history
```

### Making it automatic

To have every `git` command you type automatically route through `git-explain` (instead of typing the prefix yourself), add a shell function to your `~/.bashrc` / `~/.zshrc`:

```bash
git() {
    command git-explain "$@"
}
```

This is safe from infinite recursion: `git-explain` calls the real `git` binary internally via a subprocess, which doesn't go through your shell's functions.

## How it works

`git-explain` runs your git command, and depending on what it is, takes one of two paths:

1. **Diagnosable commands** (`status`, `push`, `commit -m`, `merge`, etc.) run attached to a pseudo-terminal. Output streams live to your screen exactly as normal, and is also captured. If the command fails, that captured output is classified by the `Detector` into a specific error type, and the `Recovery` module produces an explanation and fix steps for it.

2. **Interactive commands** that need a real editor or full terminal control (`commit` with no `-m`, `rebase -i`, `add -p`, `tag -a` without `-m`) are detected up front and handed directly to `git` with your terminal's stdio inherited as-is — no interception at all. This means vi/nano/whatever editor you use behaves exactly like it normally would. The trade-off: if one of these fails, `git-explain` can't offer a diagnosis, since it never captured any output — but git's own error message is shown live either way.

## Project structure

```
git-explain/
├── src/
│   ├── main.cpp                 # entry point, wires everything together
│   ├── executor/                # runs git commands (pty relay + passthrough)
│   ├── detection/                # classifies raw git errors into ErrorType
│   ├── errors/                   # ErrorType enum + GitError struct
│   ├── recovery/                 # maps ErrorType -> explanation + fix steps
│   ├── history/                  # persists run history to disk
│   └── ui/                       # terminal output formatting (colors, layout)
├── tests/
│   ├── test_framework.hpp        # minimal TEST()/ASSERT_* framework
│   ├── test_detection.cpp
│   └── test_recovery.cpp
└── Makefile
```

## Testing

```bash
make test
```

Runs the unit test suite covering error detection (`Detector`) and fix-suggestion generation (`Recovery`). There's no automated coverage yet for `Executor` itself (the pty/passthrough logic), since that requires simulating a real terminal — see `Contributing` below if you'd like to help with that.

## Platform notes

- **macOS / Linux**: fully supported, including the pseudo-terminal relay for interactive output and the passthrough path for editor-driven commands.
- **Windows**: falls back to a simpler dual-pass output capture with no pseudo-terminal support. Interactive commands will not behave correctly under this path. Contributions to add proper Windows support (e.g. via ConPTY) are welcome.

## Known limitations

- The `needs_passthrough` heuristic (deciding which commands need direct terminal control) covers the common cases (`commit`, `rebase -i`, `add -p`, `tag -a`) but isn't exhaustive — some less common interactive flows may still go through the diagnosable path and behave unexpectedly.
- Error detection is pattern-matching based on git's English-language output. It won't work correctly if your git is configured to output in a different language (e.g. via `LANG`).
- No diagnosis is available for commands that go through the passthrough path, by design (see [How it works](#how-it-works)).

## Contributing

Issues and PRs welcome. A few good starting points:

- Add more `ErrorType` matchers in `src/detection/detector.cpp` (e.g. a dedicated case for GitHub's "Repository not found" push error, currently caught by the generic fallback).
- Add test coverage for `Executor`.
- Windows/ConPTY support.
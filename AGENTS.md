# AGENTS.md

## Scope

- These instructions apply to the entire repository unless a deeper `AGENTS.md` overrides them.

## C++ Naming

- Types: `PascalCase`
- Functions and methods: `camelCase`
- Local variables and parameters: `camelCase`
- Private fields: `mCamelCase`
- Constants: `kCamelCase`
- Namespaces: `lowercase`

## Refactoring Rules

- When refactoring names, update all in-scope references in the same change.
- Do not rename public APIs unless the task explicitly requires it.
- Do not rename symbols in third-party or vendored code under `engine/deps`.

## Command Rules

- Prefer `rg` for text and file search.
- Use `apply_patch` for manual file edits.
- Do not commit, amend, or create branches unless explicitly asked.
- Never run destructive git commands such as `git reset --hard` or `git checkout --` unless explicitly asked.
- Ask before running commands that download dependencies or require network access.
- Run `clang-format` when any code files are changed.

## Build And Test

- When build verification is needed, run CMake build commands outside the sandbox.
- When editing code files, run `clang-format` on the changed files before completion.

## Working Style

- Keep changes focused on the requested task.
- Preserve existing code style where it does not conflict with the rules above.
- Report any verification you ran and any verification you could not run.

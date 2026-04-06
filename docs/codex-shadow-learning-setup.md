# Codex Shadow Learning Setup

Use this plan to recreate the full setup on another computer without copying files from this machine.

## Goal

Install two global Codex skills:

- `session-knowledge-extract`
- `memory-consolidate`

Create a Codex memory store with:

- `patterns/`
- `entities/`
- `extracted-knowledge.md`

Enable repo-local playbooks with:

- `docs/playbooks/`

Add a short bootstrap to each repository `AGENTS.md` so Codex knows to consult memory and playbooks before judgment-heavy work.

## What the two global skills do

### `session-knowledge-extract`

Purpose:

- daily extraction safety net
- stages durable learnings from the current Codex session
- routes findings into `pattern`, `entity`, `playbook`, or `skip`

Writes:

- staged memory entries to `<codex-home>\memories\extracted-knowledge.md`
- draft repo playbooks to `docs/playbooks/` when a repeatable procedure is clearly present

Use it when:

- ending a work session or coding day
- you made several explicit corrections during the session
- you want today’s learnings captured before they are forgotten

### `memory-consolidate`

Purpose:

- weekly maintenance pass
- merges staged learnings into compact durable memory files
- prunes duplicates, noise, and stale entries

Updates:

- `<codex-home>\memories\patterns\`
- `<codex-home>\memories\entities\`
- pending items left in `<codex-home>\memories\extracted-knowledge.md`
- repo-local playbooks in `docs/playbooks/` when they need cleanup or deduplication

Use it when:

- doing weekly memory review
- the staging file has accumulated several entries
- memory files are getting noisy or repetitive

## 1. Find the Codex home directory

On the target machine, use the Codex home directory already used by that installation.

Typical layout:

```text
<codex-home>\
|- skills\
|- memories\
```

If the machine uses the default Windows layout, `<codex-home>` is usually:

```text
C:\Users\<username>\.codex
```

Do not hardcode the path in repo instructions. Treat `<codex-home>` as installation-specific.

## 2. Create the two global skill folders manually

Create this directory layout:

```text
<codex-home>\
\- skills\
   |- session-knowledge-extract\
   |  |- SKILL.md
   |  \- agents\
   |     \- openai.yaml
   \- memory-consolidate\
      |- SKILL.md
      \- agents\
         \- openai.yaml
```

Use the exact file contents in `Appendix A`.

## 3. Create the memory store

Under `<codex-home>\memories\`, create:

```text
<codex-home>\
\- memories\
   |- patterns\
   |- entities\
   \- extracted-knowledge.md
```

Initialize `extracted-knowledge.md` with:

```markdown
# Extracted Knowledge

Staged entries from `session-knowledge-extract` go here until `memory-consolidate` routes, merges, or discards them.
```

## 4. Prepare each repository

For every repo that should use shadow learning:

1. Ensure `docs/playbooks/` exists.
2. Add the bootstrap in `Appendix B` to `AGENTS.md`.

The repo-side result should look like:

```text
<repo>\
|- AGENTS.md
\- docs\
   \- playbooks\
```

## 5. Daily and weekly usage

Daily:

- Run `$session-knowledge-extract`
- Review what was staged in `extracted-knowledge.md`
- Keep only project- or team-specific knowledge

Weekly:

- Run `$memory-consolidate`
- Merge staged items into `patterns/` and `entities/`
- Review repo-local playbooks in `docs/playbooks/`
- Prune duplicates, stale entries, and generic advice

## 6. What belongs where

Store in `patterns/`:

- durable rules
- project conventions
- team preferences with lasting value

Store in `entities/`:

- people
- services
- systems
- environment-specific context

Store in `docs/playbooks/`:

- repeatable repo procedures
- deployment steps
- setup flows
- report generation routines

Do not store:

- generic software advice
- one-off debugging trivia
- rules better enforced by tooling

## 7. Verify the install

On the target machine, confirm:

- both skill folders exist under `<codex-home>\skills\`
- `SKILL.md` is present in each skill
- `agents\openai.yaml` is present in each skill
- `patterns/`, `entities/`, and `extracted-knowledge.md` exist under `<codex-home>\memories\`
- the repo has `docs/playbooks/`
- the repo `AGENTS.md` includes the shadow-learning bootstrap

Then start a Codex session in the repo and invoke:

```text
$session-knowledge-extract
```

If the skill appears and stages output successfully, the install is working.

## Appendix A: Global Skill Files

### File: `<codex-home>\skills\session-knowledge-extract\SKILL.md`

````markdown
---
name: session-knowledge-extract
description: Use when the user wants a daily extraction pass, session learnings captured, or explicit corrections and operational procedures staged into Codex memory. Triggers on requests such as "session knowledge extract", "extract learnings", "capture what we learned today", or end-of-day memory review.
---

# Session Knowledge Extract

## Overview

Extract project-specific knowledge from the current Codex session and stage it for later review. Write reusable rules to the active Codex memory root's `extracted-knowledge.md` and write repeatable repository procedures to `docs\playbooks\` in the active git repo.

## Inputs To Trust

Use only evidence that is actually visible:

- The current conversation, especially explicit corrections and `@remember` notes
- Local repo evidence such as `git status`, recent commits, plans, notes, and changed docs
- Existing memory files under the active Codex memory root

Do not invent earlier session details that are not visible in the current thread or local files.

## Step 1: Ensure the memory layout exists

Use this layout under the active Codex memory root:

```text
<memory-root>\
|- patterns\
|- entities\
|- extracted-knowledge.md
```

Create missing directories or the staging file before writing output.

## Step 2: Gather candidate knowledge

Look for high-signal material only:

- Explicit user corrections: "don't do X, do Y"
- Project or team preferences: "we use X", "always", "never", "prefer"
- Named people, services, or systems with durable context
- Repeatable procedures narrated as ordered steps

Ignore generic software advice the model already knows.

## Step 3: Classify each finding

Use this routing:

| Destination | Use for                                                    |
| ----------- | ---------------------------------------------------------- |
| `pattern`   | Reusable project or team rule, preference, or calibration  |
| `entity`    | Person, service, system, environment, or stateful actor    |
| `playbook`  | Repeatable repo-local procedure with meaningful step order |
| `skip`      | Generic advice, one-off trivia, or unsupported inference   |

Rules:

- Prefer `skip` when the value is unclear.
- Save only things a strong engineer on another team might do differently.
- For playbooks, require at least 3 meaningful steps or one clearly risky sequence.

## Step 4: Stage memory entries

Append staged entries to the active Codex memory root's `extracted-knowledge.md`.

Use this format:

```markdown
## YYYY-MM-DD

- type: pattern
  scope: <project or team>
  summary: <one-line rule>
  detail: <concise explanation and reason if known>
  source: <conversation, git, docs, or explicit remember note>
```

Guidelines:

- Group entries under today's date.
- Keep summaries short and reusable.
- Do not write duplicates if the same rule already exists in the staging file or destination files.

## Step 5: Write repo-local playbooks when justified

Only do this when the current working directory is inside a git repo.

Write playbooks to `docs\playbooks\` in the active repository. Create the directory if needed.

Use draft frontmatter:

```markdown
---
source: extracted
status: draft
updated: YYYY-MM-DD
---
```

Then write:

- Purpose
- Preconditions
- Ordered steps
- Verification
- Risks or rollback notes when relevant

Do not create playbooks for one-off debugging sessions or trivial one-command tasks.

## Step 6: Return a review summary

Report:

- Patterns staged
- Entities staged
- Playbooks created or updated
- Items intentionally skipped

If evidence is too weak, say so and stage nothing.

## Constraints

- Treat this as a daily safety net, not a license to dump everything into memory.
- Prefer explicit corrections over inferred preferences.
- Keep playbooks repo-local; do not copy them into global memory.
- Do not claim persistence until the files were actually written.
````

### File: `<codex-home>\skills\session-knowledge-extract\agents\openai.yaml`

```yaml
interface:
  display_name: "Session Knowledge Extract"
  short_description: "Extract daily session learnings"
  default_prompt: "Use $session-knowledge-extract to stage daily patterns, entities, and playbooks from recent Codex work."
```

### File: `<codex-home>\skills\memory-consolidate\SKILL.md`

````markdown
---
name: memory-consolidate
description: Use when the user wants a weekly memory review, staged learnings merged into durable files, or noisy Codex memory pruned and tightened. Triggers on requests such as "memory consolidate", "review memory", "merge staged learnings", or weekly shadow-learning maintenance.
---

# Memory Consolidate

## Overview

Consolidate staged Codex learnings into compact durable memory files. Merge useful items from the active Codex memory root's `extracted-knowledge.md` into `patterns\` and `entities\`, prune noise, and review repo-local playbooks without copying them into global memory.

## Step 1: Read the memory store first

Inspect:

- `extracted-knowledge.md` in the active Codex memory root
- Existing files under the active Codex memory root's `patterns\`
- Existing files under the active Codex memory root's `entities\`
- `docs\playbooks\` in the active repo if it exists

If the staging file does not exist or is empty, say so and stop unless the user explicitly asks for broader cleanup.

## Step 2: Promote staged items deliberately

For each staged entry:

- Merge with an existing file when the topic already exists
- Create a new file only when a distinct topic has clearly emerged
- Rewrite for concision instead of appending raw notes
- Drop stale, generic, or duplicate entries

## Step 3: Keep files compact

Use these targets:

- `MEMORY.md`-style index files stay short if you later add them
- Pattern and entity files should stay comfortably scannable
- Prefer multiple focused files over one large dumping ground

Good pattern files contain:

- Stable rules
- Preferred approaches with brief reasons
- High-signal examples only when needed

Bad pattern files contain:

- Generic engineering advice
- Temporary task notes
- Repeated versions of the same rule

## Step 4: Review playbooks separately

Playbooks belong to the current repo, not to global memory.

If `docs\playbooks\` exists:

- Merge duplicate playbooks
- Tighten wording and step order
- Keep `source` and `status` frontmatter accurate
- Leave drafts as `draft` unless the user explicitly confirms they are reviewed

Do not move playbook content into `patterns\` unless it has become a true reusable rule rather than a procedure.

## Step 5: Clean the staging file

After promoting or intentionally discarding entries, update `extracted-knowledge.md` in the active Codex memory root so it reflects what is still pending review.

Prefer one of these outcomes:

- Remove promoted entries entirely
- Leave unresolved entries under a small `Pending review` section

Do not leave stale duplicates behind.

## Output format

Return a concise maintenance summary:

- Patterns updated or created
- Entities updated or created
- Playbooks reviewed
- Entries discarded and why
- Anything that still needs human review

## Constraints

- Memory is for project- and team-specific judgment, not textbook advice.
- Facts and stable procedures are safer than style preferences; prefer them when in doubt.
- Keep the store small enough that future Codex runs can consult it quickly.
- If a rule can be enforced deterministically by tooling, prefer tooling and keep memory focused on judgment.
````

### File: `<codex-home>\skills\memory-consolidate\agents\openai.yaml`

```yaml
interface:
  display_name: "Memory Consolidate"
  short_description: "Review and merge staged memory"
  default_prompt: "Use $memory-consolidate to merge staged Codex memory into concise pattern and entity files."
```

## Appendix B: Repo `AGENTS.md` Bootstrap

Add this section to every repository `AGENTS.md` that should use shadow learning:

```markdown
## Shadow Learning

- This repo uses Codex shadow learning.
- Before work that involves judgment such as review, architecture, planning, or writing, read the active Codex memory root's `patterns\*.md` and `entities\*.md` files when they exist.
- Read `docs\playbooks\*.md` in this repo for repeatable project procedures when they exist.
- When the user corrects you or states a durable preference, acknowledge it explicitly in the conversation so `session-knowledge-extract` can capture it later.
- Keep memory concise and project-specific. Do not store generic software advice that the model already knows.
```

# Website Navigation Map

## Initial Navigation Section (18 tabs total to first input)
1. [Homepage]
2. [go to user (email)]
3. [go to campaign]
4. [go to Task Batch]
5. [Tags]
6. [In progress/Complete]
7. [View Comments]
8. [View Instructions]
9. [Open profile menu]
10. [More options]
11. [Annotations]

### Clip Section Pattern (When Empty)
Each clip section has 5 tabs when the text field is empty:
1. [Copy all text]
2. [Edit stamps]
3. [Delete time annotation]
4. [View Comments]
5. [multi-line text input field]

### Clip Section Pattern (After Text Entry)
Each clip section expands to 7 tabs once text is entered:
1. [Copy all text]
2. [Edit stamps]
3. [Delete time annotation]
4. [View Comments]
5. [rewrite with Grammarly]    <!-- Only appears after text entry -->
6. [open Grammarly]            <!-- Only appears after text entry -->
7. [multi-line text input field]

### Formula for Navigation During Initial Typing
- First clip starts at tab 18
- Each additional clip adds 5 tabs (not 7, since Grammarly options are not present)
- For N clips, the last clip's text field will be at: 18 + (5 * (N-1))

[Rest of video controls section remains the same...]

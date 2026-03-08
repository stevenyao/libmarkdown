# Markdown Test Document

This is a comprehensive test document covering all CommonMark elements.

## Headings

# Heading 1
## Heading 2
### Heading 3
#### Heading 4
##### Heading 5
###### Heading 6

## Thematic Break

---

## Paragraphs

This is a paragraph with **bold** and *italic* and `inline code`.

This is another paragraph with ~~strikethrough~~ text.

## Code Blocks

### Fenced Code Block

```python
def hello():
    print("Hello, World!")
    return 0
```

```javascript
function greet(name) {
    console.log(`Hello, ${name}!`);
}
```

### Indented Code Block

    function test() {
        return true;
    }

## Blockquotes

> This is a blockquote.
> It can span multiple lines.

> ## Blockquote with Heading
>
> And some **formatting** inside.

## Lists

### Unordered List

- Item 1
- Item 2
- Item 3

### Nested Unordered List

- Level 1
  - Level 2
    - Level 3

### Ordered List

1. First item
2. Second item
3. Third item

### Task List (GFM)

- [x] Completed task
- [ ] Pending task
- [X] Another completed

## Links

[Link to Google](https://www.google.com)

[Link with title](https://example.com "Example Title")

## Images

![Alt text](image.png)

![Image with title](logo.jpg "Logo Title")

## Autolinks (GFM)

<https://www.google.com>

<email@example.com>

## Emphasis

*Italic text*

_Also italic_

**Bold text**

__Also bold__

***Bold and italic***

___Bold and italic___

~~Strikethrough~~

## Inline Code

Here is some `inline code` and more `code here`.

## Tables (GFM)

| Header 1 | Header 2 | Header 3 |
|----------|----------|----------|
| Cell 1   | Cell 2   | Cell 3   |
| Cell 4   | Cell 5   | Cell 6   |

| Left | Center | Right |
|:-----|:------:|------:|
| L    |   C    |     R |

## HTML Blocks

<div>
    This is an HTML block.
</div>

## Inline HTML

This is <span>inline HTML</span> text.

## Line Breaks

This is line one  
This is line two

## Link Reference Definitions

[foo]: https://example.com "Foo"
[bar]: https://example2.com

[foo] and [bar] are reference links.

---

End of test document.

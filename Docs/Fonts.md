# Font JSON Spec

The *.font files under Assets/Fonts use the following JSON keys:

- `chars`: A string listing each character in the font, in the order they appear.
- `width`: The texture sheet width of each character. This does NOT determine the display width, only where each character is on the texture sheet.
- `total_height`: The height of the texture sheet, including the area below the baseline.
- `baseline`: The height of the baseline from the top of the texture sheet.
- `char_spacing`: The horizontal spacing between characters.
- `line_spacing`: The vertical spacing between lines.
- `space_width`: The width of the space character.
- `default_char_width`: The default width of characters. This is used for characters not in the `char_widths` list.
- `default_size`: The default size of the font. This should be the same as `baseline`.
- `texture`: The texture sheet for the font. This should be the basename of the generated asset file, minus the extension, for example `interface_font`.
- `uppercase_only`: Set to true if this font contains no lowercase letters.
- `char_widths`: A dictionary of character widths, where the keys are the character, and the value is their width. Any char without a key here will use `default_char_width`.

All fields are required.
"""HTML sanitizer for Gruyere, a web application with holes.

Copyright 2017 Google Inc. All rights reserved.

This code is licensed under the https://creativecommons.org/licenses/by-nd/3.0/us/
Creative Commons Attribution-No Derivative Works 3.0 United States license.

DO NOT COPY THIS CODE!

This application is a small self-contained web application with numerous
security holes. It is provided for use with the Web Application Exploits and
Defenses codelab. You may modify the code for your own use while doing the
codelab but you may not distribute the modified code. Brief excerpts of this
code may be used for educational or instructional purposes provided this
notice is kept intact. By using Gruyere you agree to the Terms of Service
https://www.google.com/intl/en/policies/terms/
"""

__author__ = 'Bruce Leban'

# system modules
import re


SAFE_COLOR_RE = re.compile(r"^#?[a-zA-Z0-9]*$")  # TODO: Regex for color


# TODO Function to sanitize color inputs
def _SanitizeColor(color):
  """Sanitizes a color, returning 'invalid' if it's invalid.

  A valid value is either the name of a color or # followed by the
  hex code for a color (like #FEFFFF). Returning an invalid value
  value allows a style sheet to specify a default value by writing
  'color:default; color:{{foo:color}}'.
  """

  if SAFE_COLOR_RE.match(color):
    return color
  return 'invalid'


def SanitizeJs(s):
  """Sanitizes JS input"""
  return s.lower().replace('script', 'span')


def SanitizeHtml(s):
  """Makes html safe for embedding in a document.

  Filters the html to exclude all but a small subset of html by
  removing script tags/attributes.

  Args:
    s: some html to sanitize.

  Returns:
    The html with all unsafe html removed.
  """
  processed = ''
  while s:
    start = s.find('<')
    if start >= 0:
      end = s.find('>', start)
      if end >= 0:
        before = s[:start]
        tag = s[start:end+1]
        after = s[end+1:]
      else:
        before = s[:start]
        tag = s[start:]
        after = ''
    else:
      before = s
      tag = ''
      after = ''

    processed += before + _SanitizeTag(tag)
    s = after
  return processed


TAG_RE = re.compile(r'<(.*?)(\s|>)')  # matches the start of an html tag


def _SanitizeTag(t):
  """Sanitizes a single html tag.

  This does both a 'whitelist' for
  the allowed tags and a 'blacklist' for the disallowed attributes.

  Args:
    t: a tag to sanitize.

  Returns:
    a safe tag.
  """
  allowed_tags = [
      'a', 'b', 'big', 'br', 'center', 'code', 'em', 'h1', 'h2', 'h3',
      'h4', 'h5', 'h6', 'hr', 'i', 'img', 'li', 'ol', 'p', 's', 'small',
      'span', 'strong', 'table', 'td', 'tr', 'u', 'ul',
  ]
  disallowed_attributes = [
      'onblur', 'onchange', 'onclick', 'ondblclick', 'onfocus',
      'onkeydown', 'onkeypress', 'onkeyup', 'onload', 'onmousedown',
      'onmousemove', 'onmouseout', 'onmouseup', 'onreset',
      'onselect', 'onsubmit', 'onunload', 'onmouseover'  # TODO fixed onmouseover
  ]

  # Extract the tag name and make sure it's allowed.
  if t.startswith('</'):
    mt = TAG_RE.match(t.replace('</', '<').lower())  # TODO Ensure also ending tags match allowed tags
    if mt is None:
      return t
  m = TAG_RE.match(t.replace('</', '<').lower())  # TODO lowercase
  if m is None:
    return t
  tag_name = m.group(1)
  if tag_name.lower() not in allowed_tags:  # TODO Ensure case insensitive
    t = t[:m.start(1)] + 'blocked' + t[m.end(1):]

  # This is a bit heavy handed but we want to be sure we don't
  # allow any to get through.
  for a in disallowed_attributes:
    t = t.lower().replace(a, 'blocked')  # TODO Ensure case insensitive
  return t

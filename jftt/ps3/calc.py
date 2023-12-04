# -----------------------------------------------------------------------------
# calc.py
# -----------------------------------------------------------------------------

from sly import Lexer, Parser

Z = 1234577

def normalise(x):
  return ((x % Z) + Z) % Z


def add(x, y):
  return normalise(normalise(x) + normalise(y))


def subtract(x, y):
  return normalise(normalise(x) - normalise(y))

def multiply(x, y):
  return normalise(normalise(x) * normalise(y))


def inverse(a):
  m = Z
  x = 1
  y = 0

  while a > 1:
    quotient = a / m
    t = m

    m = a % m
    a = t
    t = y

    y = x - quotient * y
    x = t

  if x < 0:
    x += Z

  return x

def divide_unchecked(x, y):
  inv = inverse(y)
  return multiply(x, inv)

def mod_unchecked(x, y):
  return normalise(normalise(x) % normalise(y))

def power(x, y):
  x = normalise(x)
  result = 1
  for i in range(0, y):
    result = normalise(result * x)
  return result

def unary_minus(x):
  return normalise(-x)

class CalcLexer(Lexer):
  tokens = {
    COMMENT,
    NUMBER,
    PLUS,
    TIMES,
    MINUS,
    DIVIDE,
    PERCENT,
    HAT,
    LPAREN,
    RPAREN,
    NEWLINE,
  }

  ignore  = ' \t'
  ignore_extension = r'\\\n'

  COMMENT = r'#(.|\\\n)*'
  NUMBER  = r'\d+'
  PLUS    = r'\+'
  MINUS   = r'-'
  TIMES   = r'\*'
  DIVIDE  = r'/'
  PERCENT = r'%'
  HAT     = r'\^'
  LPAREN  = r'\('
  RPAREN  = r'\)'
  NEWLINE = r'\n'

  def PLUS(self, token):
    return token

  def NUMBER(self, token):
    token.value = int(token.value)
    return token

  def error(self, t):
    print("illegal character '%s'" % t.value[0])
    self.index += 1

class CalcParser(Parser):
  debugfile = "debug.out"

  tokens = CalcLexer.tokens

  precedence = (
    ('left', PLUS, MINUS),
    ('left', TIMES, DIVIDE, PERCENT),
    ('right', HAT),
    ('right', UMINUS),
  )

  def __init__(self):
    self.encountered_error = False
    self.rpn = ""

  def append_rpn(self, value):
    self.rpn += str(value)
    self.rpn += " ";

  def print_error(self, msg):
    if self.encountered_error == False:
      print(f"error: {msg}")

  def error(self, t):
    print(f"error: token '{t}'")

  # @_('entry statement NEWLINE')
  # def entry(self, p):
  #   pass

  # @_('entry NEWLINE', '')
  # def entry(self, p):
  #   pass

  @_('expr NEWLINE')
  def statement(self, p):
    print(self.rpn)
    self.rpn = ""
    if(self.encountered_error == False):
      print(p.expr)
    self.encountered_error = False
    return p.expr

  # @_('COMMENT')
  # def statement(self, p):
  #   pass

  @_('expr PLUS expr')
  def expr(self, p):
    self.append_rpn("+")
    return add(p.expr0, p.expr1)

  @_('expr MINUS expr')
  def expr(self, p):
    self.append_rpn("-")
    return subtract(p.expr0, p.expr1)

  @_('expr TIMES expr')
  def expr(self, p):
    self.append_rpn("*")
    return multiply(p.expr0, p.expr1)

  @_('expr DIVIDE expr')
  def expr(self, p):
    self.append_rpn("/")
    if p.expr1 == 0:
      self.print_error("division by 0")
      self.encountered_error = True

    return divide_unchecked(p.expr0, p.expr1)

  @_('expr PERCENT expr')
  def expr(self, p):
    self.append_rpn("%")
    if p.expr1 == 0:
      self.print_error("division by 0")
      self.encountered_error = True

    return mod_unchecked(p.expr0, p.expr1)

  @_('expr HAT term')
  def expr(self, p):
    self.append_rpn("^")
    return power(p.expr0, p.expr1)

  @_('LPAREN expr RPAREN')
  def expr(self, p):
    return p.expr

  @_('term')
  def expr(self, p):
    return p.term

  @_('MINUS NUMBER %prec UMINUS')
  def term(self, p):
    value = unary_minus(p.NUMBER)
    self.append_rpn(value)
    return value

  @_('NUMBER')
  def term(self, p):
    value = normalise(p.NUMBER)
    self.append_rpn(value)
    return value

if __name__ == '__main__':
  lexer = CalcLexer()
  parser = CalcParser()
  while True:
    try:
      text = input('> ')
    except EOFError:
      break
    if text:
      tokens = lexer.tokenize(text)
      parser.parse(tokens)

Vim Cheatsheet

Disclaimer: This cheatsheet is summarized from personal experience and other online tutorials. It should not be considered as an official advice.

Global

    :help keyword # 打开关键字帮助
    :o file       # 打开文件
    :saveas file  # 另存为
    :close        # 另存为

Cursor movement

    h        # 左移光标
    j        # 下移光标
    k        # 上移光标
    l        # 右移光标
    H        # 移动到当前页面顶部
    M        # 移动到当前页面中间
    L        # 移动到当前页面底部
    w        # 移动到下个单词开头
    W        # 移动到下个单词开头(单词含标点)
    e        # 移动到下个单词结尾
    E        # 移动到下个单词结尾(单词含标点)
    b        # 移动到上个单词结尾
    B        # 移动到上个单词结尾(单词含标点)
    0        # 移动到行首
    ^        # 移动到行首的非空白符
    $        # 移动到行尾
    g_       # 移动到行内最后一个非空白符
    gg       # 移动到文件第一行
    G        # 移动到文件最后一行
    5G       # 移动到第五行
    fx       # 动到字符 x 下次出现的位置
    tx       # 移动到字符 x 下次出现的位置的前一个字符
    }        # jump to next paragraph (or function/block, when editing code)
    {        # jump to previous paragraph (or function/block, when editing code)
    zz       # 移动屏幕使光标居中
    Ctrl + b # 向后滚动一屏
    Ctrl + f # 向前滚动一屏
    Ctrl + d # 向前滚动半屏
    Ctrl + u # 向后滚动半屏

Insert mode - inserting/appending text

    cnt        # 从光标前开始插入字符
    I        # 从行首开始插入字符
    a        # 从光标后开始插入字符
    A        # 从行尾开始插入字符
    o        # 在当前行之下另起一行, 开始插入字符
    O        # 在当前行之上另起一行, 开始插入字符
    ea       # 从当前单词末尾开始插入
    Esc      # 退出插入模式

Editing

    r        # 替换当前字符
    J        # 将下一行合并到当前行
    cc       # 清空当前行, 然后进入插入模式
    cw       # 从光标位置开始, 修改单词
    ce       # change (replace) to the end of the next word
    cb       # change (replace) to the start of the previous word
    c$       # 从光标位置开始, 修改当前行
    s        # 删除当前字符, 然后进入插入模式
    S        # delete line and substitute text (same as cc)
    xp       # transpose two letters (delete and paste)
    .        # repeat last command
    u        # undo
    Ctrl + r # redo

Marking text (visual mode)

    v        # start visual mode, mark lines, then do a command (like y-yank)
    V        # start linewise visual mode
    o        # move to other end of marked area
    O        # move to other corner of block
    aw       # mark a word
    ab       # a block with ()
    aB       # a block with {}
    ib       # inner block with ()
    iB       # inner block with {}
    Esc      # exit visual mode
    Ctrl + v # start visual block mode

Visual commands

    >       # shift text right
    <       # shift text left
    y       # yank (copy) marked text
    d       # delete marked text
    ~       # switch case

Cut and paste

    yy       # yank (copy) a line
    2yy      # yank (copy) 2 lines
    yw       # yank (copy) the characters of the word from the cursor position to the start of the next word
    y$       # yank (copy) to end of line
    p        # put (paste) the clipboard after cursor
    P        # put (paste) before cursor
    dd       # delete (cut) a line
    2dd      # delete (cut) 2 lines
    dw       # delete (cut) the characters of the word from the cursor position to the start of the next word
    D        # delete (cut) to the end of the line
    d$       # delete (cut) to the end of the line
    d^       # delete (cut) to the first non-blank character of the line
    d0       # delete (cut) to the begining of the line
    x        # delete (cut) character

Search and replace

    /pattern       # search for pattern
    ?pattern       # search backward for pattern
    \vpattern      # 'very magic' pattern: non-alphanumeric characters are interpreted as special regex symbols (no escaping needed)
    n              # repeat search in same direction
    N              # repeat search in opposite direction
    :%s/old/new/g  # replace all old with new throughout file
    :%s/old/new/gc # replace all old with new throughout file with confirmations
    :noh           # remove highlighting of search matches

Search in multiple files

    :vimgrep /pattern/ {file} # search for pattern in multiple files
    :cn                       # jump to the next match
    :cp                       # jump to the previous match
    :copen                    # open a window containing the list of matches

Exiting

    :w              # write (save) the file, but don't exit
    :w !sudo tee %  # write out the current file using sudo
    :wq or :x or ZZ # write (save) and quit
    :q              # quit (fails if there are unsaved changes)
    :q! or ZQ       # quit and throw away unsaved changes

Working with multiple files

    :e file       # edit a file in a new buffer
    :bnext or :bn # go to the next buffer
    :bprev or :bp # go to the previous buffer
    :bd           # delete a buffer (close a file)
    :ls           # list all open buffers
    :sp file      # open a file in a new buffer and split window
    :vsp file     # open a file in a new buffer and vertically split window
    Ctrl + ws     # split window
    Ctrl + ww     # switch windows
    Ctrl + wq     # quit a window
    Ctrl + wv     # split window vertically
    Ctrl + wh     # move cursor to the left window (vertical split)
    Ctrl + wl     # move cursor to the right window (vertical split)
    Ctrl + wj     # move cursor to the window below (horizontal split)
    Ctrl + wk     # move cursor to the window above (horizontal split)

Tabs

    :tabnew or :tabnew file # open a file in a new tab
    Ctrl + wT               # move the current split window into its own tab
    gt or :tabnext or :tabn # move to the next tab
    gT or :tabprev or :tabp # move to the previous tab
    <number>gt              # move to tab <number>
    :tabmove <number>       # move current tab to the <number>th position (indexed from 0)
    :tabclose or :tabc      # close the current tab and all its windows
    :tabonly or :tabo       # close all tabs except for the current one
    :tabdo command          # run the command on all tabs (e.g. :tabdo q - closes all opened tabs)


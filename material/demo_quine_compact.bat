@echo off

rem Do't forget to use the Quine mode for the literals

rem Target file
gabuzomeu --file="quine_compact.gbzm" --target="quine_compact_t.gbzm" --Quine

rem Target console
rem Care : in this case there are two extra's : "1020 birds babble : " at begin and "new line" at end
rem gabuzomeu --file="quine_compact.gbzm" > quine_compact_t.gbzm --Quine

fc /b quine_compact.gbzm quine_compact_t.gbzm
@echo off
setlocal enabledelayedexpansion

for /r %%f in (*) do (
	    for /f "tokens=1" %%h in ('certutil -hashfile "%%f" MD5 ^| find /i /v "hash" ^| find /i /v "certutil"') do (
		            if defined seen[%%h] (
				                echo Deleting duplicate: %%f
						            del "%%f"
							            ) else (
									                set seen[%%h]=1
											        )
												    )
											    )


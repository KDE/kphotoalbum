# SPDX-FileCopyrightText: 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

_checks[check_recover-from-idzero]="Recovery from tags with id=0"
_context[check_recover-from-idzero]="<h2>What this test will do:</h2>
<p><ul>
<li>This test checks whether a database file with tags with id=0 is read correctly.<br/>
For a detailed analysis on how this kind of database inconsistency could occur, see <a href='https://bugs.kde.org/show_bug.cgi?id=415415'>bug #415415</a>
</li>
<li>After you close this dialog, KPhotoAlbum will be started 2 times in a row.</li>
</ul>
</p>
<h2>What you have to do:</h2>
<ol>
<li>Each time KPhotoAlbum starts, save the database and exit KPhotoAlbum.</li>
</ol>"
check_recover-from-idzero()
{
	local check_name="check_recover-from-idzero"
	local check_dir="$TEMPDIR/$check_name"
	local data_dir="$mydir/db/recover-from-idzero"
	setup_check "$check_dir" || return $result_err_setup
	# not needed in this scenario:
	rm -r "$check_dir/db"

	kdialog --msgbox "<h1>$check_name</h1>${_context[$check_name]}"

	for subcheck in compressed uncompressed
	do
		local subcheck_dir="$check_dir/$subcheck"
		mkdir "$subcheck_dir" || return $result_err_setup

		local add_rc="[General]\nuntaggedCategory=\nuntaggedTag=\n"
		if [[ "$subcheck" == "uncompressed" ]]
		then
			add_rc="$add_rc\nuseCompressedIndexXML=false\n"
		fi
		echo -e "$add_rc$BASE_RC" > "$subcheck_dir/kphotoalbumrc" || return $result_err_setup

		# prepare database:
		cp "$data_dir/$subcheck.orig.xml" "$subcheck_dir/index.xml" || return $result_err_setup

		export XDG_CONFIG_HOME="$subcheck_dir"
		kphotoalbum --db "$subcheck_dir/index.xml" > "$subcheck_dir/log" 2>&1 || return $result_err_crash

		if ! diff -u "$data_dir/$subcheck.result.xml" "$subcheck_dir/index.xml"
		then
			log notice "$check_name/$subcheck: Mismatch in index.xml!"
			return $result_failed
		fi

		sed -i "s:$TEMPDIR:IMAGEROOT:g" "$subcheck_dir/log"
		if ! diff -u "$data_dir/$subcheck.expected.log" "$subcheck_dir/log"
		then
			log notice "$check_name/$subcheck: Mismatch in log messages!"
			return $result_failed
		fi
	done
	return $result_ok
}

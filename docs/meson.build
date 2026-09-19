scdoc = find_program('scdoc', required: true)

if scdoc.found()
    custom_target(
        'manpage',
        input: 'labline.1.scd',
        output: 'labline.1',
        command: [scdoc],
        feed: true,
        capture: true,
        build_by_default: true,
        install: true,
        install_dir: get_option('mandir') / 'man1'
    )
endif

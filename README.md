# SS Proj

This school project was created for the [SS (Sistemski Softver, en. System Software) course][ss], which is part of the Bachelor's studies at the [School of Electrical Engineering][school], [University of Belgrade][uni].

This project consisted of two parts:

 1. an assembler for the [pre-defined assembly language][arch]. The two pass assembler generates a custom object file which can be relocated and linked as needed.
 2. an emulator for a [finctional architecture][arch] (defined in the project statement). The interpretive emulator loads a single object file and executes it, while supporting interrupts and i/o byte streams as defined in the statement.

 The [project statement][statement] along with the [definition of the fictional architecture][arch] is given in the `docs/statement` folder. The final reports with short descriptions of the [assembler][1_report] and [emulator][2_report] are given in the `docs/report` folder. All the docs are in Servian.

The `tests` folder contains different inputs (assembler code for the first part, obj files for the second) for showcasing the capabilities of the solution.

[ss]: http://si3ss.etf.rs/
[school]: https://www.etf.bg.ac.rs/
[uni]: https://www.bg.ac.rs/
[statement]: ./docs/statement/DZ_SP_SS_JUN17.pdf
[arch]: ./docs/statement/microRISC.pdf
[1_report]: ./docs/report/1_assembler.pdf
[2_report]: ./docs/report/2_emulator.pdf

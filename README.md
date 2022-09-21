# BShell

BShell is a shell that allowing its users to create process bundles that contain one or more commands. After creation, when a bundle is executed, all of its processes in the bundle execute concurrently. It also supports pipeline ans file redirection operations on the bundles. A process bundle can redirect its output to a pipeline or file. All of outputs of processes in the bundle is redirected to the target. When the input comes from a file, all processes reads input file seperately. On the other hand, when isput comes from another process bundle via pipeline, the input should be replicated so that every process in the bundle can access to input.

I have provided implementation of BShell.

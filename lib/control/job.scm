;;;
;;; control.job - common job description
;;;  
;;;   Copyright (c) 2010-2011  Shiro Kawai  <shiro@acm.org>
;;;   
;;;   Redistribution and use in source and binary forms, with or without
;;;   modification, are permitted provided that the following conditions
;;;   are met:
;;;   
;;;   1. Redistributions of source code must retain the above copyright
;;;      notice, this list of conditions and the following disclaimer.
;;;  
;;;   2. Redistributions in binary form must reproduce the above copyright
;;;      notice, this list of conditions and the following disclaimer in the
;;;      documentation and/or other materials provided with the distribution.
;;;  
;;;   3. Neither the name of the authors nor the names of its contributors
;;;      may be used to endorse or promote products derived from this
;;;      software without specific prior written permission.
;;;  
;;;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
;;;   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
;;;   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
;;;   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
;;;   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;;;   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
;;;   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
;;;   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
;;;   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
;;;   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
;;;   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;;  

(define-module control.job
  (use gauche.threads)
  (use gauche.record)
  (export
   ;; API for end client
   job job? job-status job-result job-wait
   job-acknowledge-time job-start-time job-finish-time
   ;; API for control libraries
   make-job job-touch! job-acknowledge! job-run! job-mark-killed!))
(select-module control.job)

;; JOB is a common structure used by several control flow libraries
;; The use of slots are up to those libraries, and generally the end
;; user shouldn't care.
(define-record-type job %make-job #t
  (thunk)             ; the thing to do
  (specific)          ; library-specific data
  (status)            ; #f, acknowledged, running, done, error, killed.
  (result)            ; result of thunk, a condition, or the kill reason
  (waiter-cv)         ; wait on this cv for result change
  (waiter-mutex)      ; mutex to check result
  (depends-on)        ; list of jobs, when we track job dependency
  (acknowledge-time)  ; timestamp when this job is acknowledged
  (start-time)        ; timestamp when this job starts being executed
  (finish-time))      ; timestamp when this job is finished

(define (make-job thunk :key (specific #f) (waitable #f))
  (%make-job thunk specific #f #f
             (and waitable (make-condition-variable))
             (and waitable (make-mutex))
             '() #f #f #f))

(define (job-wait job :optional (timeout #f) (timeout-val #f))
  (let ([mutex (job-waiter-mutex job)]
        [cv (job-waiter-cv job)])
    (unless (and mutex cv) (error "job is not waitable" job))
    (let loop ()
      (mutex-lock! mutex)
      (let1 s (job-status job)
        (cond [(memq s '(done error killed)) (mutex-unlock! mutex) s]
              [(mutex-unlock! mutex cv timeout) (loop)]
              [else timeout-val])))))

(define (job-touch! job when :optional (now (current-time)))
  (case when
    [(:acknowledge) (job-acknowledge-time-set! job now)]
    [(:start)       (job-start-time-set! job now)]
    [(:finish)      (job-finish-time-set! job now)])
  now)

(define (job-acknowledge! job)
  (job-touch! job :acknowledge)
  (job-status-set! job 'acknowledged))

(define (job-run! job)
  (define (finish status result)
    (job-touch! job :finish)
    (job-result-set! job result)
    (job-status-set! job status) ; no hazard. I'm the only writer.
    (if-let1 cv (job-waiter-cv job)
      (condition-variable-broadcast! cv)))
  (job-status-set! job 'running)
  (job-touch! job :start)
  (guard (e [else (finish 'error e)])
    (let1 r ((job-thunk job))
      (finish 'done r))))

;; this doesn't actually kill the process/threads that running the job,
;; but merely marks the job killed.
(define (job-mark-killed! job reason)
  (job-touch! job :finish)
  (job-result-set! job reason)
  (job-status-set! job 'killed)
  (if-let1 cv (job-waiter-cv job)
    (condition-variable-broadcast! cv)))

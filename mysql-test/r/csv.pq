drop table if exists t1,t2,t3,t4;
CREATE TABLE t1 (
Period smallint(4) unsigned zerofill DEFAULT '0000' NOT NULL,
Varor_period smallint(4) unsigned DEFAULT '0' NOT NULL
) ENGINE = CSV;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
INSERT INTO t1 VALUES (9410,9412);
select period from t1;
period
9410
select * from t1;
Period	Varor_period
9410	9412
select t1.* from t1;
Period	Varor_period
9410	9412
CREATE TABLE t2 (
auto int not null,
fld1 int(6) unsigned zerofill DEFAULT '000000' NOT NULL,
companynr tinyint(2) unsigned zerofill DEFAULT '00' NOT NULL,
fld3 char(30) DEFAULT '' NOT NULL,
fld4 char(35) DEFAULT '' NOT NULL,
fld5 char(35) DEFAULT '' NOT NULL,
fld6 char(4) DEFAULT '' NOT NULL
) ENGINE = CSV;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
select t2.fld3 from t2 where companynr = 58 and fld3 like "%imaginable%";
fld3
imaginable
select fld3 from t2 where fld3 like "%cultivation" ;
fld3
cultivation
select t2.fld3,companynr from t2 where companynr = 57+1 order by fld3;
fld3	companynr
concoct	58
druggists	58
engrossing	58
Eurydice	58
exclaimers	58
ferociousness	58
hopelessness	58
Huey	58
imaginable	58
judges	58
merging	58
ostrich	58
peering	58
Phelps	58
presumes	58
Ruth	58
sentences	58
Shylock	58
straggled	58
synergy	58
thanking	58
tying	58
unlocks	58
select fld3,companynr from t2 where companynr = 58 order by fld3;
fld3	companynr
concoct	58
druggists	58
engrossing	58
Eurydice	58
exclaimers	58
ferociousness	58
hopelessness	58
Huey	58
imaginable	58
judges	58
merging	58
ostrich	58
peering	58
Phelps	58
presumes	58
Ruth	58
sentences	58
Shylock	58
straggled	58
synergy	58
thanking	58
tying	58
unlocks	58
select fld3 from t2 order by fld3 desc limit 10;
fld3
youthfulness
yelped
Wotan
workers
Witt
witchcraft
Winsett
Willy
willed
wildcats
select fld3 from t2 order by fld3 desc limit 5;
fld3
youthfulness
yelped
Wotan
workers
Witt
select fld3 from t2 order by fld3 desc limit 5,5;
fld3
witchcraft
Winsett
Willy
willed
wildcats
UPDATE t2 SET fld3="foo" WHERE fld3="b%";
select fld3 from t2;
fld3
Abraham
Aden
Adlerian
Adolph
Aldrich
Alison
Anatole
Angeles
Angles
Anna
Antarctica
Antares
Anthony
Arabia
Armour
Artemia
Asiaticizations
Augustine
Aztecan
Baird
Baltimorean
Barry
Beatrice
Beebe
Blackfoots
Blythe
Brice
Brigham
Britannic
British
Bruxelles
Buchanan
Butterfield
CERN
Caltech
Carleton
Caroline
Cassites
Chablis
Chantilly
Chicana
Chippewa
Chippewa
Choctaw
Clayton
Colombo
Colombo
Conley
Connally
Corcoran
Corinthianizes
Crays
Crosby
Cubans
Cynthia
Dar
Darius
DiMaggio
Dutchman
Edenization
Eiffel
Elmhurst
Epiphany
Ernestine
Eskimoizeds
Eulerian
Eurydice
Evanston
Everhart
Fenton
Fischbein
Fitzpatrick
Franklinizations
Galatean
Gandhian
Gandhian
Ganymede
Garrisonian
Gershwins
Goldstine
Goodrich
Gothicism
Grady
Graves
Greenberg
Gurkha
Harpy
Hawaii
Hegelian
Heusen
Hodges
Hornblower
Huey
Huffman
Hunter
Ibero-
Iranizes
Iraqis
Italianization
Italianization
Italianizes
Jobrel
Joplin
Judas
Kane
Kantian
Katherine
Kevin
Kinsey
Kline
Lagos
Lars
Latinizes
Lillian
Lizzy
Madeira
Majorca
Majorca
Manhattanize
Mardis
McGovern
Medusan
Melinda
Mendelian
Mendelssohn
Merritt
Mexican
Micronesia
Microsoft
Miles
Miltonism
Modula
Moll
Mossberg
Muscat
Nabisco
Nadia
Nazis
Newtonian
Norwalk
Omaha
Pakistani
Palestine
Pandora
Parsifal
Patrice
Persian
Peruvian
Peruvianizes
Phelps
Pinsky
Pipestone
Punjab
Puritan
Pygmalion
Pyle
Quixotism
Remus
Rena
Rochford
Romano
Romans
Ruth
Sabine
Sault
Saxony
Schoenberg
Seattle
Selfridge
Shanghais
Shylock
Simla
Simon
Stalin
Steinberg
Strauss
Tannenbaum
Taoism
Taoist
Telefunken
Teresa
Tipperary
Trobriand
Tropez
Viennese
Weissmuller
Willy
Winsett
Witt
Wotan
abates
abiding
abrogating
absentee
abut
accessed
accessed
accruing
accumulating
accuracies
acquaint
acuity
adjourn
admiring
admonishing
adsorption
advertising
aeronautic
affective
affects
affixed
afield
afloat
afore
aging
ahead
aims
airships
algorithmic
aligning
alike
alimony
allot
allowed
alphabetic
already
amateurish
amenities
ammonium
anaerobic
analog
analogy
analyzable
and
anesthetizing
animals
animized
annihilates
announced
announces
annoyers
anteater
apiary
apish
appendixes
appendixes
appendixes
appendixes
appendixes
appendixes
apprehensible
aqueous
arches
arriving
arteries
arteriole
artificially
askers
aspire
assails
assassinated
assayed
assessor
assurers
astound
attainments
attendants
attitude
attrition
audiology
automate
avenge
avoidable
awesome
aye
babies
babysitting
balled
ballgown
bankruptcies
batting
beaner
beasts
beaters
bedlam
bedpost
bee
beetles
befouled
beguiles
behavior
behaviorally
belays
bellow
belted
bestseller
betroth
betterments
bewilderingly
bibliographies
bibliography
bike
bills
births
bitterroot
bivalves
blinded
bloater
bloodbath
boasted
boat
bomb
bombast
bonfire
boom
boorish
botany
boulder
boulevard
braking
breaking
bridgeable
brokenness
bromides
bruises
brunch
buckboards
bulk
bulldozes
burglarized
burlesque
burstiness
buyer
cadenced
cage
callus
camping
cannot
capably
capped
captivated
caressing
cartographer
cascade
catalog
causality
cautioned
ceiling
celery
cemented
certificates
chafe
chamberlains
chancellor
chaperone
charcoal
charges
chasm
chastisers
checkpoints
checksumming
chess
chewing
chews
chillier
chillingly
chin
chronicle
chronography
ciphers
circumference
circus
cited
civics
civilize
clamored
cleansed
cleanser
cleanup
clenched
clerked
clinker
cloakroom
clobbering
clockers
clubroom
coarse
coexist
coinciding
cokes
colorful
combed
combinations
comfortingly
coming
commencements
commended
commercially
commits
commonplace
communicants
commuters
compartment
competitions
complex
comprehensive
comprised
conceptions
concludes
concoct
configurations
congregates
congresswoman
connective
considered
conspirator
contrary
contrasted
contributorily
contrition
controller
convenient
convulsion
cooks
corny
corset
cosy
count
counterpoise
coverings
crazed
craziness
creak
creator
creditably
creek
critiques
cross
crowder
cruelty
crunches
crushers
crying
culled
cult
cultivation
cupboard
cupped
cured
curs
cute
daughter
deaf
decaying
decliner
decoding
decomposition
deductions
definiteness
dehydrate
deludes
demand
democracy
demultiplex
den
denies
denizen
denotative
denounces
dental
dentally
deputies
descendants
despising
despot
destroyer
detained
detectably
devices
dialysis
difficultly
diffusing
dilutes
dimensions
dipole
dire
disable
disappointing
disconnects
discounts
disentangle
dislocate
disobedience
disobedience
displacement
displease
dissociate
distinguish
distort
distracting
dogging
dopers
drains
dreaded
drovers
druggists
duality
dub
dubbed
ducks
dull
dusted
eastbound
effortlessly
electroencephalography
elite
embassies
emergency
employing
encompass
encompasses
endpoint
engrossing
environing
environing
epistle
equestrian
equilibrium
erases
error
eschew
estimating
eternal
evened
evenhandedly
eventful
excises
excitation
exclaim
exclaim
exclaimers
exclamation
excrete
exhausts
exhibits
expelled
expiring
explorers
exponentials
extents
exterminated
externally
extracted
extramarital
faced
faithful
famine
fanatic
fangs
far
fated
fatty
featherweight
features
feed
feminine
ferociousness
fetched
fetters
fevers
fibrosities
fifteenth
fiftieth
filial
fingerings
finishers
firearm
fitting
fixedly
flanking
fleeing
flint
flog
flopping
floundered
flurried
flyers
foldout
folksong
foothill
forgivably
forgivably
forked
forthcoming
founder
fourscore
fragile
freakish
freest
freezes
funereal
furnishings
furthermore
gadfly
gainers
gainful
galling
garage
garages
gated
gazer
gems
generalized
generically
genres
gentle
gentleman
getters
gifted
glacial
gleaning
glut
goblins
golfing
goodness
governing
gradually
grammatic
grayness
grazing
gritty
groupings
guides
guitars
gutting
halting
hamming
hand
handgun
handsomest
handy
heals
heaving
heiress
helmsman
hen
hewed
hoarder
hoarder
hollowly
homeless
homeowner
honeybee
honeysuckle
honoring
hooker
hopelessness
hostess
however
humanness
humidly
humiliation
humility
hushes
husky
hydrant
hypothesizer
icon
icon
ideas
imaginable
imaginable
imagining
impelling
impending
imperial
imperiously
implicated
implied
imprecisely
imprecision
imprint
impulsive
inaccuracy
inch
incidentals
incorrectly
incurring
index
indulge
indulgences
ineffective
inexperienced
infallibly
infatuate
infest
inform
inheritresses
inking
inmate
innuendo
innumerable
insolence
institutes
instruments
intelligibility
intentness
interactions
intercepted
interdependent
interrelationships
interrogate
invalids
investigations
irresponsibly
irritates
jarred
jarring
jewelry
journalizing
joyfully
judges
juveniles
kanji
kept
kerosene
kingdom
kiting
label
labeled
labeled
lacerating
landslide
languages
laterally
lawgiver
leaflet
leaning
leavings
lectured
leftover
lemons
leopards
lewdly
libretto
lied
lights
linear
lists
lithograph
loaded
lockings
lockout
lore
luckily
machining
males
marginal
mastering
mastering
masterpiece
mate
maternal
mayoral
meanwhile
measures
measures
mechanics
mechanizing
medical
meditation
megabytes
merging
merry
meson
metaphysically
millivolt
mineral
miniaturizes
minima
minion
minting
mirror
mishaps
misted
mistiness
misunderstander
mixture
modulators
monochromatic
moribund
motors
mournfulness
mouthful
multilayer
mumbles
mushrooms
mutations
mutton
mystic
narrowed
navally
navies
navigate
neat
neonatal
nested
nonbiodegradable
noncritical
nondecreasing
normalizes
north
northeaster
nouns
nullified
numerals
nymph
obliterates
obnoxious
observations
obtain
occupancies
occurred
offerers
offload
omnisciently
opaquely
ores
organizer
orphanage
ostrich
outwitting
overestimating
overlay
palmed
panty
parametrized
paranoid
parenthood
parters
participated
partridges
passport
pastimes
patenting
patriots
peacock
peeked
peering
pellagra
percentage
percentage
persist
perturb
pessimist
pests
petted
pharmaceutic
philosophic
phony
photographers
photosensitive
physicists
pictures
pitfalls
pithed
pithing
pityingly
planar
poison
poisoning
polka
population
portrayal
posed
positioning
postcondition
postulation
pragmatism
praised
precaution
precession
precipitable
preclude
presentation
preserve
pressure
presumes
previewing
priceless
priding
primary
principled
profundity
provers
provers
psychic
publicly
puddings
quagmire
qualify
quelling
quiets
quitter
quotes
racial
rackets
railway
raining
rains
rainstorm
ransomer
rats
ravines
readable
realest
realized
realtor
reassigned
receivers
recording
recruited
reduce
reexamines
refiner
regimented
regimented
registration
relatively
relaxes
relaxing
relegated
relics
relishing
relives
reminiscence
renew
repelled
repetitions
reporters
reporters
repressions
resplendent
restlessly
resumes
retrace
retransmitting
reuniting
reviver
rifles
rightful
rightfully
rightfulness
ripeness
riser
rooms
roped
ropers
roundly
rouses
rubies
rudeness
rules
rural
rusting
sadly
sags
salvaging
sanding
saplings
sash
sating
saucepans
save
saving
sawtooth
scalps
scarf
scars
scatter
scatterbrain
scheduling
schemer
scholastics
scholastics
scolds
scornfully
scotch
scribbled
search
secures
securing
semblance
seminaries
sentences
serializations
seriously
serpents
serums
serving
severely
sews
sexually
shapelessly
shipyard
shooter
shouted
shrieks
shuttered
sicker
signaled
signifying
silk
silverware
similarities
simultaneous
sinking
skies
skips
skulking
slaughter
slaughtered
sleepwalk
sloping
smoothed
snatching
socializes
solidification
sophomore
sorters
southwest
spates
spatial
spear
specification
specifics
spiders
spies
spiritual
spongers
spools
sportswriting
sporty
squabbled
squeaking
squeezer
squeezes
stabilizes
staffing
stairway
standardizes
star
starlet
starlet
stated
stealth
stickers
stiller
stint
stodgy
stopped
store
straggled
straight
stranglings
strategic
stratifications
stratified
strokes
stuck
subdirectories
subdirectory
subdivision
subjective
submarines
subschema
succumbed
suffocate
sugars
suite
suites
sumac
superstitious
suppliers
sureties
surreptitious
survey
suspend
swaying
sweats
sweetish
swelling
syndicate
synergy
t1
tabernacle
tape
tapestry
target
taxed
taxonomically
technically
techniques
teem
teethe
televise
tempering
terminal
terminator
terminators
test
testicle
textures
thanking
theorizers
therefore
throttles
thugs
tidiness
timesharing
tinily
tinnily
tinting
title
title
tits
titter
toggle
towering
towers
tragedies
traitor
transcontinental
transferals
transistorizing
translatable
trimmings
troop
tropics
true
trumps
tucked
tutor
tying
unaffected
uncles
uncovering
undiscovered
undoes
unexpected
unfulfilled
ungrateful
uniform
uninterruptedly
universals
university
unlocks
unnerves
unplug
unruly
untidiness
untying
unwilling
uprisings
vacuuming
validate
vanish
variance
ventilate
veranda
verify
versatility
vest
vests
violinist
visualized
voltage
wallet
waltz
warbling
warm
warningly
watering
weakening
weasels
weekly
welcoming
western
wetness
whistled
whiteners
whiteness
widens
wildcats
willed
witchcraft
workers
yelped
youthfulness
UPDATE t2 SET fld3="bar" WHERE fld3="s%";
select fld3 from t2;
fld3
Abraham
Aden
Adlerian
Adolph
Aldrich
Alison
Anatole
Angeles
Angles
Anna
Antarctica
Antares
Anthony
Arabia
Armour
Artemia
Asiaticizations
Augustine
Aztecan
Baird
Baltimorean
Barry
Beatrice
Beebe
Blackfoots
Blythe
Brice
Brigham
Britannic
British
Bruxelles
Buchanan
Butterfield
CERN
Caltech
Carleton
Caroline
Cassites
Chablis
Chantilly
Chicana
Chippewa
Chippewa
Choctaw
Clayton
Colombo
Colombo
Conley
Connally
Corcoran
Corinthianizes
Crays
Crosby
Cubans
Cynthia
Dar
Darius
DiMaggio
Dutchman
Edenization
Eiffel
Elmhurst
Epiphany
Ernestine
Eskimoizeds
Eulerian
Eurydice
Evanston
Everhart
Fenton
Fischbein
Fitzpatrick
Franklinizations
Galatean
Gandhian
Gandhian
Ganymede
Garrisonian
Gershwins
Goldstine
Goodrich
Gothicism
Grady
Graves
Greenberg
Gurkha
Harpy
Hawaii
Hegelian
Heusen
Hodges
Hornblower
Huey
Huffman
Hunter
Ibero-
Iranizes
Iraqis
Italianization
Italianization
Italianizes
Jobrel
Joplin
Judas
Kane
Kantian
Katherine
Kevin
Kinsey
Kline
Lagos
Lars
Latinizes
Lillian
Lizzy
Madeira
Majorca
Majorca
Manhattanize
Mardis
McGovern
Medusan
Melinda
Mendelian
Mendelssohn
Merritt
Mexican
Micronesia
Microsoft
Miles
Miltonism
Modula
Moll
Mossberg
Muscat
Nabisco
Nadia
Nazis
Newtonian
Norwalk
Omaha
Pakistani
Palestine
Pandora
Parsifal
Patrice
Persian
Peruvian
Peruvianizes
Phelps
Pinsky
Pipestone
Punjab
Puritan
Pygmalion
Pyle
Quixotism
Remus
Rena
Rochford
Romano
Romans
Ruth
Sabine
Sault
Saxony
Schoenberg
Seattle
Selfridge
Shanghais
Shylock
Simla
Simon
Stalin
Steinberg
Strauss
Tannenbaum
Taoism
Taoist
Telefunken
Teresa
Tipperary
Trobriand
Tropez
Viennese
Weissmuller
Willy
Winsett
Witt
Wotan
abates
abiding
abrogating
absentee
abut
accessed
accessed
accruing
accumulating
accuracies
acquaint
acuity
adjourn
admiring
admonishing
adsorption
advertising
aeronautic
affective
affects
affixed
afield
afloat
afore
aging
ahead
aims
airships
algorithmic
aligning
alike
alimony
allot
allowed
alphabetic
already
amateurish
amenities
ammonium
anaerobic
analog
analogy
analyzable
and
anesthetizing
animals
animized
annihilates
announced
announces
annoyers
anteater
apiary
apish
appendixes
appendixes
appendixes
appendixes
appendixes
appendixes
apprehensible
aqueous
arches
arriving
arteries
arteriole
artificially
askers
aspire
assails
assassinated
assayed
assessor
assurers
astound
attainments
attendants
attitude
attrition
audiology
automate
avenge
avoidable
awesome
aye
babies
babysitting
balled
ballgown
bankruptcies
batting
beaner
beasts
beaters
bedlam
bedpost
bee
beetles
befouled
beguiles
behavior
behaviorally
belays
bellow
belted
bestseller
betroth
betterments
bewilderingly
bibliographies
bibliography
bike
bills
births
bitterroot
bivalves
blinded
bloater
bloodbath
boasted
boat
bomb
bombast
bonfire
boom
boorish
botany
boulder
boulevard
braking
breaking
bridgeable
brokenness
bromides
bruises
brunch
buckboards
bulk
bulldozes
burglarized
burlesque
burstiness
buyer
cadenced
cage
callus
camping
cannot
capably
capped
captivated
caressing
cartographer
cascade
catalog
causality
cautioned
ceiling
celery
cemented
certificates
chafe
chamberlains
chancellor
chaperone
charcoal
charges
chasm
chastisers
checkpoints
checksumming
chess
chewing
chews
chillier
chillingly
chin
chronicle
chronography
ciphers
circumference
circus
cited
civics
civilize
clamored
cleansed
cleanser
cleanup
clenched
clerked
clinker
cloakroom
clobbering
clockers
clubroom
coarse
coexist
coinciding
cokes
colorful
combed
combinations
comfortingly
coming
commencements
commended
commercially
commits
commonplace
communicants
commuters
compartment
competitions
complex
comprehensive
comprised
conceptions
concludes
concoct
configurations
congregates
congresswoman
connective
considered
conspirator
contrary
contrasted
contributorily
contrition
controller
convenient
convulsion
cooks
corny
corset
cosy
count
counterpoise
coverings
crazed
craziness
creak
creator
creditably
creek
critiques
cross
crowder
cruelty
crunches
crushers
crying
culled
cult
cultivation
cupboard
cupped
cured
curs
cute
daughter
deaf
decaying
decliner
decoding
decomposition
deductions
definiteness
dehydrate
deludes
demand
democracy
demultiplex
den
denies
denizen
denotative
denounces
dental
dentally
deputies
descendants
despising
despot
destroyer
detained
detectably
devices
dialysis
difficultly
diffusing
dilutes
dimensions
dipole
dire
disable
disappointing
disconnects
discounts
disentangle
dislocate
disobedience
disobedience
displacement
displease
dissociate
distinguish
distort
distracting
dogging
dopers
drains
dreaded
drovers
druggists
duality
dub
dubbed
ducks
dull
dusted
eastbound
effortlessly
electroencephalography
elite
embassies
emergency
employing
encompass
encompasses
endpoint
engrossing
environing
environing
epistle
equestrian
equilibrium
erases
error
eschew
estimating
eternal
evened
evenhandedly
eventful
excises
excitation
exclaim
exclaim
exclaimers
exclamation
excrete
exhausts
exhibits
expelled
expiring
explorers
exponentials
extents
exterminated
externally
extracted
extramarital
faced
faithful
famine
fanatic
fangs
far
fated
fatty
featherweight
features
feed
feminine
ferociousness
fetched
fetters
fevers
fibrosities
fifteenth
fiftieth
filial
fingerings
finishers
firearm
fitting
fixedly
flanking
fleeing
flint
flog
flopping
floundered
flurried
flyers
foldout
folksong
foothill
forgivably
forgivably
forked
forthcoming
founder
fourscore
fragile
freakish
freest
freezes
funereal
furnishings
furthermore
gadfly
gainers
gainful
galling
garage
garages
gated
gazer
gems
generalized
generically
genres
gentle
gentleman
getters
gifted
glacial
gleaning
glut
goblins
golfing
goodness
governing
gradually
grammatic
grayness
grazing
gritty
groupings
guides
guitars
gutting
halting
hamming
hand
handgun
handsomest
handy
heals
heaving
heiress
helmsman
hen
hewed
hoarder
hoarder
hollowly
homeless
homeowner
honeybee
honeysuckle
honoring
hooker
hopelessness
hostess
however
humanness
humidly
humiliation
humility
hushes
husky
hydrant
hypothesizer
icon
icon
ideas
imaginable
imaginable
imagining
impelling
impending
imperial
imperiously
implicated
implied
imprecisely
imprecision
imprint
impulsive
inaccuracy
inch
incidentals
incorrectly
incurring
index
indulge
indulgences
ineffective
inexperienced
infallibly
infatuate
infest
inform
inheritresses
inking
inmate
innuendo
innumerable
insolence
institutes
instruments
intelligibility
intentness
interactions
intercepted
interdependent
interrelationships
interrogate
invalids
investigations
irresponsibly
irritates
jarred
jarring
jewelry
journalizing
joyfully
judges
juveniles
kanji
kept
kerosene
kingdom
kiting
label
labeled
labeled
lacerating
landslide
languages
laterally
lawgiver
leaflet
leaning
leavings
lectured
leftover
lemons
leopards
lewdly
libretto
lied
lights
linear
lists
lithograph
loaded
lockings
lockout
lore
luckily
machining
males
marginal
mastering
mastering
masterpiece
mate
maternal
mayoral
meanwhile
measures
measures
mechanics
mechanizing
medical
meditation
megabytes
merging
merry
meson
metaphysically
millivolt
mineral
miniaturizes
minima
minion
minting
mirror
mishaps
misted
mistiness
misunderstander
mixture
modulators
monochromatic
moribund
motors
mournfulness
mouthful
multilayer
mumbles
mushrooms
mutations
mutton
mystic
narrowed
navally
navies
navigate
neat
neonatal
nested
nonbiodegradable
noncritical
nondecreasing
normalizes
north
northeaster
nouns
nullified
numerals
nymph
obliterates
obnoxious
observations
obtain
occupancies
occurred
offerers
offload
omnisciently
opaquely
ores
organizer
orphanage
ostrich
outwitting
overestimating
overlay
palmed
panty
parametrized
paranoid
parenthood
parters
participated
partridges
passport
pastimes
patenting
patriots
peacock
peeked
peering
pellagra
percentage
percentage
persist
perturb
pessimist
pests
petted
pharmaceutic
philosophic
phony
photographers
photosensitive
physicists
pictures
pitfalls
pithed
pithing
pityingly
planar
poison
poisoning
polka
population
portrayal
posed
positioning
postcondition
postulation
pragmatism
praised
precaution
precession
precipitable
preclude
presentation
preserve
pressure
presumes
previewing
priceless
priding
primary
principled
profundity
provers
provers
psychic
publicly
puddings
quagmire
qualify
quelling
quiets
quitter
quotes
racial
rackets
railway
raining
rains
rainstorm
ransomer
rats
ravines
readable
realest
realized
realtor
reassigned
receivers
recording
recruited
reduce
reexamines
refiner
regimented
regimented
registration
relatively
relaxes
relaxing
relegated
relics
relishing
relives
reminiscence
renew
repelled
repetitions
reporters
reporters
repressions
resplendent
restlessly
resumes
retrace
retransmitting
reuniting
reviver
rifles
rightful
rightfully
rightfulness
ripeness
riser
rooms
roped
ropers
roundly
rouses
rubies
rudeness
rules
rural
rusting
sadly
sags
salvaging
sanding
saplings
sash
sating
saucepans
save
saving
sawtooth
scalps
scarf
scars
scatter
scatterbrain
scheduling
schemer
scholastics
scholastics
scolds
scornfully
scotch
scribbled
search
secures
securing
semblance
seminaries
sentences
serializations
seriously
serpents
serums
serving
severely
sews
sexually
shapelessly
shipyard
shooter
shouted
shrieks
shuttered
sicker
signaled
signifying
silk
silverware
similarities
simultaneous
sinking
skies
skips
skulking
slaughter
slaughtered
sleepwalk
sloping
smoothed
snatching
socializes
solidification
sophomore
sorters
southwest
spates
spatial
spear
specification
specifics
spiders
spies
spiritual
spongers
spools
sportswriting
sporty
squabbled
squeaking
squeezer
squeezes
stabilizes
staffing
stairway
standardizes
star
starlet
starlet
stated
stealth
stickers
stiller
stint
stodgy
stopped
store
straggled
straight
stranglings
strategic
stratifications
stratified
strokes
stuck
subdirectories
subdirectory
subdivision
subjective
submarines
subschema
succumbed
suffocate
sugars
suite
suites
sumac
superstitious
suppliers
sureties
surreptitious
survey
suspend
swaying
sweats
sweetish
swelling
syndicate
synergy
t1
tabernacle
tape
tapestry
target
taxed
taxonomically
technically
techniques
teem
teethe
televise
tempering
terminal
terminator
terminators
test
testicle
textures
thanking
theorizers
therefore
throttles
thugs
tidiness
timesharing
tinily
tinnily
tinting
title
title
tits
titter
toggle
towering
towers
tragedies
traitor
transcontinental
transferals
transistorizing
translatable
trimmings
troop
tropics
true
trumps
tucked
tutor
tying
unaffected
uncles
uncovering
undiscovered
undoes
unexpected
unfulfilled
ungrateful
uniform
uninterruptedly
universals
university
unlocks
unnerves
unplug
unruly
untidiness
untying
unwilling
uprisings
vacuuming
validate
vanish
variance
ventilate
veranda
verify
versatility
vest
vests
violinist
visualized
voltage
wallet
waltz
warbling
warm
warningly
watering
weakening
weasels
weekly
welcoming
western
wetness
whistled
whiteners
whiteness
widens
wildcats
willed
witchcraft
workers
yelped
youthfulness
DELETE FROM t2 WHERE fld3="r%";
SELECT fld3 FROM t2;
fld3
Abraham
Aden
Adlerian
Adolph
Aldrich
Alison
Anatole
Angeles
Angles
Anna
Antarctica
Antares
Anthony
Arabia
Armour
Artemia
Asiaticizations
Augustine
Aztecan
Baird
Baltimorean
Barry
Beatrice
Beebe
Blackfoots
Blythe
Brice
Brigham
Britannic
British
Bruxelles
Buchanan
Butterfield
CERN
Caltech
Carleton
Caroline
Cassites
Chablis
Chantilly
Chicana
Chippewa
Chippewa
Choctaw
Clayton
Colombo
Colombo
Conley
Connally
Corcoran
Corinthianizes
Crays
Crosby
Cubans
Cynthia
Dar
Darius
DiMaggio
Dutchman
Edenization
Eiffel
Elmhurst
Epiphany
Ernestine
Eskimoizeds
Eulerian
Eurydice
Evanston
Everhart
Fenton
Fischbein
Fitzpatrick
Franklinizations
Galatean
Gandhian
Gandhian
Ganymede
Garrisonian
Gershwins
Goldstine
Goodrich
Gothicism
Grady
Graves
Greenberg
Gurkha
Harpy
Hawaii
Hegelian
Heusen
Hodges
Hornblower
Huey
Huffman
Hunter
Ibero-
Iranizes
Iraqis
Italianization
Italianization
Italianizes
Jobrel
Joplin
Judas
Kane
Kantian
Katherine
Kevin
Kinsey
Kline
Lagos
Lars
Latinizes
Lillian
Lizzy
Madeira
Majorca
Majorca
Manhattanize
Mardis
McGovern
Medusan
Melinda
Mendelian
Mendelssohn
Merritt
Mexican
Micronesia
Microsoft
Miles
Miltonism
Modula
Moll
Mossberg
Muscat
Nabisco
Nadia
Nazis
Newtonian
Norwalk
Omaha
Pakistani
Palestine
Pandora
Parsifal
Patrice
Persian
Peruvian
Peruvianizes
Phelps
Pinsky
Pipestone
Punjab
Puritan
Pygmalion
Pyle
Quixotism
Remus
Rena
Rochford
Romano
Romans
Ruth
Sabine
Sault
Saxony
Schoenberg
Seattle
Selfridge
Shanghais
Shylock
Simla
Simon
Stalin
Steinberg
Strauss
Tannenbaum
Taoism
Taoist
Telefunken
Teresa
Tipperary
Trobriand
Tropez
Viennese
Weissmuller
Willy
Winsett
Witt
Wotan
abates
abiding
abrogating
absentee
abut
accessed
accessed
accruing
accumulating
accuracies
acquaint
acuity
adjourn
admiring
admonishing
adsorption
advertising
aeronautic
affective
affects
affixed
afield
afloat
afore
aging
ahead
aims
airships
algorithmic
aligning
alike
alimony
allot
allowed
alphabetic
already
amateurish
amenities
ammonium
anaerobic
analog
analogy
analyzable
and
anesthetizing
animals
animized
annihilates
announced
announces
annoyers
anteater
apiary
apish
appendixes
appendixes
appendixes
appendixes
appendixes
appendixes
apprehensible
aqueous
arches
arriving
arteries
arteriole
artificially
askers
aspire
assails
assassinated
assayed
assessor
assurers
astound
attainments
attendants
attitude
attrition
audiology
automate
avenge
avoidable
awesome
aye
babies
babysitting
balled
ballgown
bankruptcies
batting
beaner
beasts
beaters
bedlam
bedpost
bee
beetles
befouled
beguiles
behavior
behaviorally
belays
bellow
belted
bestseller
betroth
betterments
bewilderingly
bibliographies
bibliography
bike
bills
births
bitterroot
bivalves
blinded
bloater
bloodbath
boasted
boat
bomb
bombast
bonfire
boom
boorish
botany
boulder
boulevard
braking
breaking
bridgeable
brokenness
bromides
bruises
brunch
buckboards
bulk
bulldozes
burglarized
burlesque
burstiness
buyer
cadenced
cage
callus
camping
cannot
capably
capped
captivated
caressing
cartographer
cascade
catalog
causality
cautioned
ceiling
celery
cemented
certificates
chafe
chamberlains
chancellor
chaperone
charcoal
charges
chasm
chastisers
checkpoints
checksumming
chess
chewing
chews
chillier
chillingly
chin
chronicle
chronography
ciphers
circumference
circus
cited
civics
civilize
clamored
cleansed
cleanser
cleanup
clenched
clerked
clinker
cloakroom
clobbering
clockers
clubroom
coarse
coexist
coinciding
cokes
colorful
combed
combinations
comfortingly
coming
commencements
commended
commercially
commits
commonplace
communicants
commuters
compartment
competitions
complex
comprehensive
comprised
conceptions
concludes
concoct
configurations
congregates
congresswoman
connective
considered
conspirator
contrary
contrasted
contributorily
contrition
controller
convenient
convulsion
cooks
corny
corset
cosy
count
counterpoise
coverings
crazed
craziness
creak
creator
creditably
creek
critiques
cross
crowder
cruelty
crunches
crushers
crying
culled
cult
cultivation
cupboard
cupped
cured
curs
cute
daughter
deaf
decaying
decliner
decoding
decomposition
deductions
definiteness
dehydrate
deludes
demand
democracy
demultiplex
den
denies
denizen
denotative
denounces
dental
dentally
deputies
descendants
despising
despot
destroyer
detained
detectably
devices
dialysis
difficultly
diffusing
dilutes
dimensions
dipole
dire
disable
disappointing
disconnects
discounts
disentangle
dislocate
disobedience
disobedience
displacement
displease
dissociate
distinguish
distort
distracting
dogging
dopers
drains
dreaded
drovers
druggists
duality
dub
dubbed
ducks
dull
dusted
eastbound
effortlessly
electroencephalography
elite
embassies
emergency
employing
encompass
encompasses
endpoint
engrossing
environing
environing
epistle
equestrian
equilibrium
erases
error
eschew
estimating
eternal
evened
evenhandedly
eventful
excises
excitation
exclaim
exclaim
exclaimers
exclamation
excrete
exhausts
exhibits
expelled
expiring
explorers
exponentials
extents
exterminated
externally
extracted
extramarital
faced
faithful
famine
fanatic
fangs
far
fated
fatty
featherweight
features
feed
feminine
ferociousness
fetched
fetters
fevers
fibrosities
fifteenth
fiftieth
filial
fingerings
finishers
firearm
fitting
fixedly
flanking
fleeing
flint
flog
flopping
floundered
flurried
flyers
foldout
folksong
foothill
forgivably
forgivably
forked
forthcoming
founder
fourscore
fragile
freakish
freest
freezes
funereal
furnishings
furthermore
gadfly
gainers
gainful
galling
garage
garages
gated
gazer
gems
generalized
generically
genres
gentle
gentleman
getters
gifted
glacial
gleaning
glut
goblins
golfing
goodness
governing
gradually
grammatic
grayness
grazing
gritty
groupings
guides
guitars
gutting
halting
hamming
hand
handgun
handsomest
handy
heals
heaving
heiress
helmsman
hen
hewed
hoarder
hoarder
hollowly
homeless
homeowner
honeybee
honeysuckle
honoring
hooker
hopelessness
hostess
however
humanness
humidly
humiliation
humility
hushes
husky
hydrant
hypothesizer
icon
icon
ideas
imaginable
imaginable
imagining
impelling
impending
imperial
imperiously
implicated
implied
imprecisely
imprecision
imprint
impulsive
inaccuracy
inch
incidentals
incorrectly
incurring
index
indulge
indulgences
ineffective
inexperienced
infallibly
infatuate
infest
inform
inheritresses
inking
inmate
innuendo
innumerable
insolence
institutes
instruments
intelligibility
intentness
interactions
intercepted
interdependent
interrelationships
interrogate
invalids
investigations
irresponsibly
irritates
jarred
jarring
jewelry
journalizing
joyfully
judges
juveniles
kanji
kept
kerosene
kingdom
kiting
label
labeled
labeled
lacerating
landslide
languages
laterally
lawgiver
leaflet
leaning
leavings
lectured
leftover
lemons
leopards
lewdly
libretto
lied
lights
linear
lists
lithograph
loaded
lockings
lockout
lore
luckily
machining
males
marginal
mastering
mastering
masterpiece
mate
maternal
mayoral
meanwhile
measures
measures
mechanics
mechanizing
medical
meditation
megabytes
merging
merry
meson
metaphysically
millivolt
mineral
miniaturizes
minima
minion
minting
mirror
mishaps
misted
mistiness
misunderstander
mixture
modulators
monochromatic
moribund
motors
mournfulness
mouthful
multilayer
mumbles
mushrooms
mutations
mutton
mystic
narrowed
navally
navies
navigate
neat
neonatal
nested
nonbiodegradable
noncritical
nondecreasing
normalizes
north
northeaster
nouns
nullified
numerals
nymph
obliterates
obnoxious
observations
obtain
occupancies
occurred
offerers
offload
omnisciently
opaquely
ores
organizer
orphanage
ostrich
outwitting
overestimating
overlay
palmed
panty
parametrized
paranoid
parenthood
parters
participated
partridges
passport
pastimes
patenting
patriots
peacock
peeked
peering
pellagra
percentage
percentage
persist
perturb
pessimist
pests
petted
pharmaceutic
philosophic
phony
photographers
photosensitive
physicists
pictures
pitfalls
pithed
pithing
pityingly
planar
poison
poisoning
polka
population
portrayal
posed
positioning
postcondition
postulation
pragmatism
praised
precaution
precession
precipitable
preclude
presentation
preserve
pressure
presumes
previewing
priceless
priding
primary
principled
profundity
provers
provers
psychic
publicly
puddings
quagmire
qualify
quelling
quiets
quitter
quotes
racial
rackets
railway
raining
rains
rainstorm
ransomer
rats
ravines
readable
realest
realized
realtor
reassigned
receivers
recording
recruited
reduce
reexamines
refiner
regimented
regimented
registration
relatively
relaxes
relaxing
relegated
relics
relishing
relives
reminiscence
renew
repelled
repetitions
reporters
reporters
repressions
resplendent
restlessly
resumes
retrace
retransmitting
reuniting
reviver
rifles
rightful
rightfully
rightfulness
ripeness
riser
rooms
roped
ropers
roundly
rouses
rubies
rudeness
rules
rural
rusting
sadly
sags
salvaging
sanding
saplings
sash
sating
saucepans
save
saving
sawtooth
scalps
scarf
scars
scatter
scatterbrain
scheduling
schemer
scholastics
scholastics
scolds
scornfully
scotch
scribbled
search
secures
securing
semblance
seminaries
sentences
serializations
seriously
serpents
serums
serving
severely
sews
sexually
shapelessly
shipyard
shooter
shouted
shrieks
shuttered
sicker
signaled
signifying
silk
silverware
similarities
simultaneous
sinking
skies
skips
skulking
slaughter
slaughtered
sleepwalk
sloping
smoothed
snatching
socializes
solidification
sophomore
sorters
southwest
spates
spatial
spear
specification
specifics
spiders
spies
spiritual
spongers
spools
sportswriting
sporty
squabbled
squeaking
squeezer
squeezes
stabilizes
staffing
stairway
standardizes
star
starlet
starlet
stated
stealth
stickers
stiller
stint
stodgy
stopped
store
straggled
straight
stranglings
strategic
stratifications
stratified
strokes
stuck
subdirectories
subdirectory
subdivision
subjective
submarines
subschema
succumbed
suffocate
sugars
suite
suites
sumac
superstitious
suppliers
sureties
surreptitious
survey
suspend
swaying
sweats
sweetish
swelling
syndicate
synergy
t1
tabernacle
tape
tapestry
target
taxed
taxonomically
technically
techniques
teem
teethe
televise
tempering
terminal
terminator
terminators
test
testicle
textures
thanking
theorizers
therefore
throttles
thugs
tidiness
timesharing
tinily
tinnily
tinting
title
title
tits
titter
toggle
towering
towers
tragedies
traitor
transcontinental
transferals
transistorizing
translatable
trimmings
troop
tropics
true
trumps
tucked
tutor
tying
unaffected
uncles
uncovering
undiscovered
undoes
unexpected
unfulfilled
ungrateful
uniform
uninterruptedly
universals
university
unlocks
unnerves
unplug
unruly
untidiness
untying
unwilling
uprisings
vacuuming
validate
vanish
variance
ventilate
veranda
verify
versatility
vest
vests
violinist
visualized
voltage
wallet
waltz
warbling
warm
warningly
watering
weakening
weasels
weekly
welcoming
western
wetness
whistled
whiteners
whiteness
widens
wildcats
willed
witchcraft
workers
yelped
youthfulness
DELETE FROM t2 WHERE fld3="d%" ORDER BY RAND();
SELECT fld3 FROM t2;
fld3
Abraham
Aden
Adlerian
Adolph
Aldrich
Alison
Anatole
Angeles
Angles
Anna
Antarctica
Antares
Anthony
Arabia
Armour
Artemia
Asiaticizations
Augustine
Aztecan
Baird
Baltimorean
Barry
Beatrice
Beebe
Blackfoots
Blythe
Brice
Brigham
Britannic
British
Bruxelles
Buchanan
Butterfield
CERN
Caltech
Carleton
Caroline
Cassites
Chablis
Chantilly
Chicana
Chippewa
Chippewa
Choctaw
Clayton
Colombo
Colombo
Conley
Connally
Corcoran
Corinthianizes
Crays
Crosby
Cubans
Cynthia
Dar
Darius
DiMaggio
Dutchman
Edenization
Eiffel
Elmhurst
Epiphany
Ernestine
Eskimoizeds
Eulerian
Eurydice
Evanston
Everhart
Fenton
Fischbein
Fitzpatrick
Franklinizations
Galatean
Gandhian
Gandhian
Ganymede
Garrisonian
Gershwins
Goldstine
Goodrich
Gothicism
Grady
Graves
Greenberg
Gurkha
Harpy
Hawaii
Hegelian
Heusen
Hodges
Hornblower
Huey
Huffman
Hunter
Ibero-
Iranizes
Iraqis
Italianization
Italianization
Italianizes
Jobrel
Joplin
Judas
Kane
Kantian
Katherine
Kevin
Kinsey
Kline
Lagos
Lars
Latinizes
Lillian
Lizzy
Madeira
Majorca
Majorca
Manhattanize
Mardis
McGovern
Medusan
Melinda
Mendelian
Mendelssohn
Merritt
Mexican
Micronesia
Microsoft
Miles
Miltonism
Modula
Moll
Mossberg
Muscat
Nabisco
Nadia
Nazis
Newtonian
Norwalk
Omaha
Pakistani
Palestine
Pandora
Parsifal
Patrice
Persian
Peruvian
Peruvianizes
Phelps
Pinsky
Pipestone
Punjab
Puritan
Pygmalion
Pyle
Quixotism
Remus
Rena
Rochford
Romano
Romans
Ruth
Sabine
Sault
Saxony
Schoenberg
Seattle
Selfridge
Shanghais
Shylock
Simla
Simon
Stalin
Steinberg
Strauss
Tannenbaum
Taoism
Taoist
Telefunken
Teresa
Tipperary
Trobriand
Tropez
Viennese
Weissmuller
Willy
Winsett
Witt
Wotan
abates
abiding
abrogating
absentee
abut
accessed
accessed
accruing
accumulating
accuracies
acquaint
acuity
adjourn
admiring
admonishing
adsorption
advertising
aeronautic
affective
affects
affixed
afield
afloat
afore
aging
ahead
aims
airships
algorithmic
aligning
alike
alimony
allot
allowed
alphabetic
already
amateurish
amenities
ammonium
anaerobic
analog
analogy
analyzable
and
anesthetizing
animals
animized
annihilates
announced
announces
annoyers
anteater
apiary
apish
appendixes
appendixes
appendixes
appendixes
appendixes
appendixes
apprehensible
aqueous
arches
arriving
arteries
arteriole
artificially
askers
aspire
assails
assassinated
assayed
assessor
assurers
astound
attainments
attendants
attitude
attrition
audiology
automate
avenge
avoidable
awesome
aye
babies
babysitting
balled
ballgown
bankruptcies
batting
beaner
beasts
beaters
bedlam
bedpost
bee
beetles
befouled
beguiles
behavior
behaviorally
belays
bellow
belted
bestseller
betroth
betterments
bewilderingly
bibliographies
bibliography
bike
bills
births
bitterroot
bivalves
blinded
bloater
bloodbath
boasted
boat
bomb
bombast
bonfire
boom
boorish
botany
boulder
boulevard
braking
breaking
bridgeable
brokenness
bromides
bruises
brunch
buckboards
bulk
bulldozes
burglarized
burlesque
burstiness
buyer
cadenced
cage
callus
camping
cannot
capably
capped
captivated
caressing
cartographer
cascade
catalog
causality
cautioned
ceiling
celery
cemented
certificates
chafe
chamberlains
chancellor
chaperone
charcoal
charges
chasm
chastisers
checkpoints
checksumming
chess
chewing
chews
chillier
chillingly
chin
chronicle
chronography
ciphers
circumference
circus
cited
civics
civilize
clamored
cleansed
cleanser
cleanup
clenched
clerked
clinker
cloakroom
clobbering
clockers
clubroom
coarse
coexist
coinciding
cokes
colorful
combed
combinations
comfortingly
coming
commencements
commended
commercially
commits
commonplace
communicants
commuters
compartment
competitions
complex
comprehensive
comprised
conceptions
concludes
concoct
configurations
congregates
congresswoman
connective
considered
conspirator
contrary
contrasted
contributorily
contrition
controller
convenient
convulsion
cooks
corny
corset
cosy
count
counterpoise
coverings
crazed
craziness
creak
creator
creditably
creek
critiques
cross
crowder
cruelty
crunches
crushers
crying
culled
cult
cultivation
cupboard
cupped
cured
curs
cute
daughter
deaf
decaying
decliner
decoding
decomposition
deductions
definiteness
dehydrate
deludes
demand
democracy
demultiplex
den
denies
denizen
denotative
denounces
dental
dentally
deputies
descendants
despising
despot
destroyer
detained
detectably
devices
dialysis
difficultly
diffusing
dilutes
dimensions
dipole
dire
disable
disappointing
disconnects
discounts
disentangle
dislocate
disobedience
disobedience
displacement
displease
dissociate
distinguish
distort
distracting
dogging
dopers
drains
dreaded
drovers
druggists
duality
dub
dubbed
ducks
dull
dusted
eastbound
effortlessly
electroencephalography
elite
embassies
emergency
employing
encompass
encompasses
endpoint
engrossing
environing
environing
epistle
equestrian
equilibrium
erases
error
eschew
estimating
eternal
evened
evenhandedly
eventful
excises
excitation
exclaim
exclaim
exclaimers
exclamation
excrete
exhausts
exhibits
expelled
expiring
explorers
exponentials
extents
exterminated
externally
extracted
extramarital
faced
faithful
famine
fanatic
fangs
far
fated
fatty
featherweight
features
feed
feminine
ferociousness
fetched
fetters
fevers
fibrosities
fifteenth
fiftieth
filial
fingerings
finishers
firearm
fitting
fixedly
flanking
fleeing
flint
flog
flopping
floundered
flurried
flyers
foldout
folksong
foothill
forgivably
forgivably
forked
forthcoming
founder
fourscore
fragile
freakish
freest
freezes
funereal
furnishings
furthermore
gadfly
gainers
gainful
galling
garage
garages
gated
gazer
gems
generalized
generically
genres
gentle
gentleman
getters
gifted
glacial
gleaning
glut
goblins
golfing
goodness
governing
gradually
grammatic
grayness
grazing
gritty
groupings
guides
guitars
gutting
halting
hamming
hand
handgun
handsomest
handy
heals
heaving
heiress
helmsman
hen
hewed
hoarder
hoarder
hollowly
homeless
homeowner
honeybee
honeysuckle
honoring
hooker
hopelessness
hostess
however
humanness
humidly
humiliation
humility
hushes
husky
hydrant
hypothesizer
icon
icon
ideas
imaginable
imaginable
imagining
impelling
impending
imperial
imperiously
implicated
implied
imprecisely
imprecision
imprint
impulsive
inaccuracy
inch
incidentals
incorrectly
incurring
index
indulge
indulgences
ineffective
inexperienced
infallibly
infatuate
infest
inform
inheritresses
inking
inmate
innuendo
innumerable
insolence
institutes
instruments
intelligibility
intentness
interactions
intercepted
interdependent
interrelationships
interrogate
invalids
investigations
irresponsibly
irritates
jarred
jarring
jewelry
journalizing
joyfully
judges
juveniles
kanji
kept
kerosene
kingdom
kiting
label
labeled
labeled
lacerating
landslide
languages
laterally
lawgiver
leaflet
leaning
leavings
lectured
leftover
lemons
leopards
lewdly
libretto
lied
lights
linear
lists
lithograph
loaded
lockings
lockout
lore
luckily
machining
males
marginal
mastering
mastering
masterpiece
mate
maternal
mayoral
meanwhile
measures
measures
mechanics
mechanizing
medical
meditation
megabytes
merging
merry
meson
metaphysically
millivolt
mineral
miniaturizes
minima
minion
minting
mirror
mishaps
misted
mistiness
misunderstander
mixture
modulators
monochromatic
moribund
motors
mournfulness
mouthful
multilayer
mumbles
mushrooms
mutations
mutton
mystic
narrowed
navally
navies
navigate
neat
neonatal
nested
nonbiodegradable
noncritical
nondecreasing
normalizes
north
northeaster
nouns
nullified
numerals
nymph
obliterates
obnoxious
observations
obtain
occupancies
occurred
offerers
offload
omnisciently
opaquely
ores
organizer
orphanage
ostrich
outwitting
overestimating
overlay
palmed
panty
parametrized
paranoid
parenthood
parters
participated
partridges
passport
pastimes
patenting
patriots
peacock
peeked
peering
pellagra
percentage
percentage
persist
perturb
pessimist
pests
petted
pharmaceutic
philosophic
phony
photographers
photosensitive
physicists
pictures
pitfalls
pithed
pithing
pityingly
planar
poison
poisoning
polka
population
portrayal
posed
positioning
postcondition
postulation
pragmatism
praised
precaution
precession
precipitable
preclude
presentation
preserve
pressure
presumes
previewing
priceless
priding
primary
principled
profundity
provers
provers
psychic
publicly
puddings
quagmire
qualify
quelling
quiets
quitter
quotes
racial
rackets
railway
raining
rains
rainstorm
ransomer
rats
ravines
readable
realest
realized
realtor
reassigned
receivers
recording
recruited
reduce
reexamines
refiner
regimented
regimented
registration
relatively
relaxes
relaxing
relegated
relics
relishing
relives
reminiscence
renew
repelled
repetitions
reporters
reporters
repressions
resplendent
restlessly
resumes
retrace
retransmitting
reuniting
reviver
rifles
rightful
rightfully
rightfulness
ripeness
riser
rooms
roped
ropers
roundly
rouses
rubies
rudeness
rules
rural
rusting
sadly
sags
salvaging
sanding
saplings
sash
sating
saucepans
save
saving
sawtooth
scalps
scarf
scars
scatter
scatterbrain
scheduling
schemer
scholastics
scholastics
scolds
scornfully
scotch
scribbled
search
secures
securing
semblance
seminaries
sentences
serializations
seriously
serpents
serums
serving
severely
sews
sexually
shapelessly
shipyard
shooter
shouted
shrieks
shuttered
sicker
signaled
signifying
silk
silverware
similarities
simultaneous
sinking
skies
skips
skulking
slaughter
slaughtered
sleepwalk
sloping
smoothed
snatching
socializes
solidification
sophomore
sorters
southwest
spates
spatial
spear
specification
specifics
spiders
spies
spiritual
spongers
spools
sportswriting
sporty
squabbled
squeaking
squeezer
squeezes
stabilizes
staffing
stairway
standardizes
star
starlet
starlet
stated
stealth
stickers
stiller
stint
stodgy
stopped
store
straggled
straight
stranglings
strategic
stratifications
stratified
strokes
stuck
subdirectories
subdirectory
subdivision
subjective
submarines
subschema
succumbed
suffocate
sugars
suite
suites
sumac
superstitious
suppliers
sureties
surreptitious
survey
suspend
swaying
sweats
sweetish
swelling
syndicate
synergy
t1
tabernacle
tape
tapestry
target
taxed
taxonomically
technically
techniques
teem
teethe
televise
tempering
terminal
terminator
terminators
test
testicle
textures
thanking
theorizers
therefore
throttles
thugs
tidiness
timesharing
tinily
tinnily
tinting
title
title
tits
titter
toggle
towering
towers
tragedies
traitor
transcontinental
transferals
transistorizing
translatable
trimmings
troop
tropics
true
trumps
tucked
tutor
tying
unaffected
uncles
uncovering
undiscovered
undoes
unexpected
unfulfilled
ungrateful
uniform
uninterruptedly
universals
university
unlocks
unnerves
unplug
unruly
untidiness
untying
unwilling
uprisings
vacuuming
validate
vanish
variance
ventilate
veranda
verify
versatility
vest
vests
violinist
visualized
voltage
wallet
waltz
warbling
warm
warningly
watering
weakening
weasels
weekly
welcoming
western
wetness
whistled
whiteners
whiteness
widens
wildcats
willed
witchcraft
workers
yelped
youthfulness
DROP TABLE t1;
ALTER TABLE t2 RENAME t1;
DROP TABLE t1;
CREATE TABLE t1 (
Period smallint(4) unsigned zerofill DEFAULT '0000' NOT NULL,
Varor_period smallint(4) unsigned DEFAULT '0' NOT NULL
) ENGINE = CSV;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
INSERT INTO t1 VALUES (9410,9412);
select period from t1;
period
9410
drop table if exists t1,t2,t3,t4;
Warnings:
Note	1051	Unknown table 'test.t2'
Note	1051	Unknown table 'test.t3'
Note	1051	Unknown table 'test.t4'
DROP TABLE IF EXISTS bug13894;
CREATE TABLE bug13894 ( val integer not null ) ENGINE = CSV;
INSERT INTO bug13894 VALUES (5);
INSERT INTO bug13894 VALUES (10);
INSERT INTO bug13894 VALUES (11);
INSERT INTO bug13894 VALUES (10);
SELECT * FROM bug13894;
val
10
10
11
5
UPDATE  bug13894 SET val=6 WHERE val=10;
SELECT * FROM bug13894;
val
11
5
6
6
DROP TABLE bug13894;
DROP TABLE IF EXISTS bug14672;
CREATE TABLE bug14672 (c1 integer not null) engine = CSV;
INSERT INTO bug14672 VALUES (1), (2), (3);
SELECT * FROM bug14672;
c1
1
2
3
DELETE FROM bug14672 WHERE c1 = 2;
SELECT * FROM bug14672;
c1
1
3
INSERT INTO bug14672 VALUES (4);
SELECT * FROM bug14672;
c1
1
3
4
INSERT INTO bug14672 VALUES (5);
SELECT * FROM bug14672;
c1
1
3
4
5
DROP TABLE bug14672;
CREATE TABLE test_concurrent_insert ( val integer not null ) ENGINE = CSV;
LOCK TABLES test_concurrent_insert READ LOCAL;
INSERT INTO test_concurrent_insert VALUES (1);
SELECT * FROM test_concurrent_insert;
val
1
SELECT * FROM test_concurrent_insert;
val
UNLOCK TABLES;
LOCK TABLES test_concurrent_insert WRITE;
INSERT INTO test_concurrent_insert VALUES (2);
SELECT * FROM test_concurrent_insert;
val
1
2
UNLOCK TABLES;
DROP TABLE test_concurrent_insert;
CREATE TABLE test_repair_table ( val integer not null ) ENGINE = CSV;
CHECK TABLE test_repair_table;
Table	Op	Msg_type	Msg_text
test.test_repair_table	check	status	OK
REPAIR TABLE test_repair_table;
Table	Op	Msg_type	Msg_text
test.test_repair_table	repair	status	OK
DROP TABLE test_repair_table;
CREATE TABLE test_repair_table2 ( val integer not null ) ENGINE = CSV;
SELECT * from test_repair_table2;
val
Error	1194	Table 'test_repair_table2' is marked as crashed and should be repaired
Warnings:
SELECT * from test_repair_table2;
val
CHECK TABLE test_repair_table2;
Table	Op	Msg_type	Msg_text
test.test_repair_table2	check	status	OK
DROP TABLE test_repair_table2;
CREATE TABLE test_repair_table3 ( val integer not null ) ENGINE = CSV;
CHECK TABLE test_repair_table3;
Table	Op	Msg_type	Msg_text
test.test_repair_table3	check	error	Corrupt
REPAIR TABLE test_repair_table3;
Table	Op	Msg_type	Msg_text
test.test_repair_table3	repair	status	OK
SELECT * FROM test_repair_table3;
val
1
4
DROP TABLE test_repair_table3;
CREATE TABLE test_repair_table4 (
num int not null,
magic_no int(4) unsigned zerofill DEFAULT '0000' NOT NULL,
company_name char(30) DEFAULT '' NOT NULL,
founded char(4) DEFAULT '' NOT NULL
) ENGINE = CSV;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
SELECT * FROM test_repair_table4;
num	magic_no	company_name	founded
Error	1194	Table 'test_repair_table4' is marked as crashed and should be repaired
Warnings:
SELECT * FROM test_repair_table4;
num	magic_no	company_name	founded
CHECK TABLE test_repair_table4;
Table	Op	Msg_type	Msg_text
test.test_repair_table4	check	status	OK
INSERT INTO test_repair_table4 VALUES (2,101,'SAP','1972');
INSERT INTO test_repair_table4 VALUES (1,101,'Microsoft','1978');
INSERT INTO test_repair_table4 VALUES (2,101,'MySQL','1995');
SELECT * FROM test_repair_table4;
num	magic_no	company_name	founded
1	0101	Microsoft	1978
2	0101	MySQL	1995
2	0101	SAP	1972
CHECK TABLE test_repair_table4;
Table	Op	Msg_type	Msg_text
test.test_repair_table4	check	status	OK
REPAIR TABLE test_repair_table4;
Table	Op	Msg_type	Msg_text
test.test_repair_table4	repair	status	OK
SELECT * FROM test_repair_table4;
num	magic_no	company_name	founded
1	0101	Microsoft	1978
2	0101	MySQL	1995
2	0101	SAP	1972
CHECK TABLE test_repair_table4;
Table	Op	Msg_type	Msg_text
test.test_repair_table4	check	status	OK
REPAIR TABLE test_repair_table4;
Table	Op	Msg_type	Msg_text
test.test_repair_table4	repair	status	OK
SELECT * FROM test_repair_table4;
num	magic_no	company_name	founded
1	0101	Microsoft	1978
2	0101	MySQL	1995
2	0101	SAP	1972
DROP TABLE test_repair_table4;
CREATE TABLE test_repair_table5 (
num int not null,
magic_no int(4) unsigned zerofill DEFAULT '0000' NOT NULL,
company_name char(30) DEFAULT '' NOT NULL,
founded char(4) DEFAULT '' NOT NULL
) ENGINE = CSV;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
CHECK TABLE test_repair_table5;
Table	Op	Msg_type	Msg_text
test.test_repair_table5	check	error	Corrupt
REPAIR TABLE test_repair_table5;
Table	Op	Msg_type	Msg_text
test.test_repair_table5	repair	status	OK
SELECT * FROM test_repair_table5;
num	magic_no	company_name	founded
INSERT INTO test_repair_table5 VALUES (1, 102, "CORRECT", 1876);
SELECT * FROM test_repair_table5;
num	magic_no	company_name	founded
1	0102	CORRECT	1876
FLUSH TABLES;
CHECK TABLE test_repair_table5;
Table	Op	Msg_type	Msg_text
test.test_repair_table5	check	error	Corrupt
REPAIR TABLE test_repair_table5;
Table	Op	Msg_type	Msg_text
test.test_repair_table5	repair	status	OK
SELECT * FROM test_repair_table5;
num	magic_no	company_name	founded
1	0102	CORRECT	1876
INSERT INTO test_repair_table5 VALUES (1, 102, "CORRECT2", 1876);
SELECT * FROM test_repair_table5;
num	magic_no	company_name	founded
1	0102	CORRECT	1876
1	0102	CORRECT2	1876
FLUSH TABLES;
CHECK TABLE test_repair_table5;
Table	Op	Msg_type	Msg_text
test.test_repair_table5	check	error	Corrupt
REPAIR TABLE test_repair_table5;
Table	Op	Msg_type	Msg_text
test.test_repair_table5	repair	status	OK
SELECT * FROM test_repair_table5;
num	magic_no	company_name	founded
1	0102	CORRECT	1876
1	0102	CORRECT2	1876
DROP TABLE test_repair_table5;
create table t1 (a int not null) engine=csv;
insert t1 values (1);
delete from t1;
affected rows: 1
delete from t1;
affected rows: 0
insert t1 values (1),(2);
delete from t1;
affected rows: 2
insert t1 values (1),(2),(3);
flush tables;
delete from t1;
affected rows: 3
insert t1 values (1),(2),(3),(4);
flush tables;
select count(*) from t1;
count(*)
4
delete from t1;
affected rows: 4
insert t1 values (1),(2),(3),(4),(5);
truncate table t1;
affected rows: 0
drop table t1;
create table t1 (v varchar(32) not null) engine=csv;
insert into t1 values ('def'),('abc'),('hij'),('3r4f');
select * from t1;
v
3r4f
abc
def
hij
alter table t1 change v v2 varchar(32) not null;
select * from t1;
v2
3r4f
abc
def
hij
alter table t1 change v2 v varchar(64) not null;
select * from t1;
v
3r4f
abc
def
hij
update t1 set v = 'lmn' where v = 'hij';
select * from t1;
v
3r4f
abc
def
lmn
alter table t1 add i int not null first;
select * from t1;
i	v
0	3r4f
0	abc
0	def
0	lmn
update t1 set i=3 where v = 'abc';
select * from t1;
i	v
0	3r4f
0	def
0	lmn
3	abc
alter table t1 change i i bigint not null;
select * from t1;
i	v
0	3r4f
0	def
0	lmn
3	abc
select * from t1 where i between 2 and 4 and v in ('def','3r4f','abc');
i	v
3	abc
drop table t1;
create table bug15205 (val int(11) not null) engine=csv;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
create table bug15205_2 (val int(11) not null) engine=csv;
Warnings:
Warning	1681	Integer display width is deprecated and will be removed in a future release.
select * from bug15205;
ERROR HY000: Can't get stat of './test/bug15205.CSV' (OS errno 2 - No such file or directory)
select * from bug15205_2;
val
select * from bug15205;
val
drop table bug15205;
drop table bug15205_2;
set names latin1;
create table t1 (
c varchar(1) not null,
name varchar(64) not null
) character set latin1 engine=csv;
insert into t1 values (0xC0,'LATIN CAPITAL LETTER A WITH GRAVE');
insert into t1 values (0xE0,'LATIN SMALL LETTER A WITH GRAVE');
insert into t1 values (0xEE,'LATIN SMALL LETTER I WITH CIRCUMFLEX');
insert into t1 values (0xFE,'LATIN SMALL LETTER THORN');
insert into t1 values (0xF7,'DIVISION SIGN');
insert into t1 values (0xFF,'LATIN SMALL LETTER Y WITH DIAERESIS');
select hex(c), c, name from t1 order by 1;
hex(c)	c	name
C0	À	LATIN CAPITAL LETTER A WITH GRAVE
E0	à	LATIN SMALL LETTER A WITH GRAVE
EE	î	LATIN SMALL LETTER I WITH CIRCUMFLEX
F7	÷	DIVISION SIGN
FE	þ	LATIN SMALL LETTER THORN
FF	ÿ	LATIN SMALL LETTER Y WITH DIAERESIS
drop table t1;
End of 5.0 tests
create table bug22080_1 (id int not null,string varchar(64) not null) Engine=CSV;
create table bug22080_2 (id int not null,string varchar(64) not null) Engine=CSV;
create table bug22080_3 (id int not null,string varchar(64) not null) Engine=CSV;
insert into bug22080_1 values(1,'string');
insert into bug22080_1 values(2,'string');
insert into bug22080_1 values(3,'string');
check table bug22080_2;
Table	Op	Msg_type	Msg_text
test.bug22080_2	check	error	Corrupt
check table bug22080_3;
Table	Op	Msg_type	Msg_text
test.bug22080_3	check	error	Corrupt
drop tables bug22080_1,bug22080_2,bug22080_3;
create table float_test (id float not null,string varchar(64) not null) Engine=CSV;
insert into float_test values(1.0,'string');
insert into float_test values(2.23,'serg.g');
insert into float_test values(0.03,'string');
insert into float_test values(0.19,'string');
insert into float_test values(.67,'string');
insert into float_test values(9.67,'string');
select * from float_test;
id	string
0.03	string
0.19	string
0.67	string
1	string
2.23	serg.g
9.67	string
drop table float_test;
create table t1(a blob not null, b int not null) engine=csv;
insert into t1 values('a', 1);
flush tables;
update t1 set b=2;
select * from t1;
a	b
a	2
drop table t1;
create table t1(a int not null) engine=csv;
insert into t1 values(-1), (-123.34), (2), (-23);
select * from t1;
a
-1
-123
-23
2
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
drop table t1;
create table t1(a int not null, b int not null) engine=csv;
repair table t1;
Table	Op	Msg_type	Msg_text
test.t1	repair	Warning	Data truncated for column 'a' at row 5
test.t1	repair	status	OK
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
select * from t1;
a	b
-1	-1
-1	-100
-200	1
1	0
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
drop table t1;
create table t1(a int not null) engine=csv;
insert into t1 values (0), (1), (2);
delete from t1 limit 2;
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
select * from t1;
a
2
delete from t1;
check table t1;
Table	Op	Msg_type	Msg_text
test.t1	check	status	OK
select * from t1;
a
drop table t1;
SET sql_mode = 'NO_ENGINE_SUBSTITUTION';
create table t1(a datetime not null) engine=csv;
insert into t1 values();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select * from t1;
a
0000-00-00 00:00:00
drop table t1;
create table t1(a set('foo','bar') not null) engine=csv;
insert into t1 values();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select * from t1;
a

drop table t1;
create table t1(a varchar(32) not null) engine=csv;
insert into t1 values();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select * from t1;
a

drop table t1;
create table t1(a int not null) engine=csv;
insert into t1 values();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select * from t1;
a
0
drop table t1;
create table t1(a blob not null) engine=csv;
insert into t1 values();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select * from t1;
a

drop table t1;
create table t1(a bit(1) not null) engine=csv;
insert into t1 values();
Warnings:
Warning	1364	Field 'a' doesn't have a default value
select BIN(a) from t1;
BIN(a)
0
drop table t1;
create table t1(a enum('foo','bar') default null) engine=csv;
ERROR 42000: The storage engine for the table doesn't support nullable columns
create table t1(a enum('foo','bar') default 'foo') engine=csv;
ERROR 42000: The storage engine for the table doesn't support nullable columns
create table t1(a enum('foo','bar') default 'foo' not null) engine=csv;
insert into t1 values();
select * from t1;
a
foo
drop table t1;
SET sql_mode = default;
CREATE TABLE t1(a INT) ENGINE=CSV;
ERROR 42000: The storage engine for the table doesn't support nullable columns
SHOW WARNINGS;
Level	Code	Message
Error	1178	The storage engine for the table doesn't support nullable columns
Error	1112	Table 't1' uses an extension that doesn't exist in this MySQL version
create table t1 (c1 tinyblob not null) engine=csv;
insert into t1 values("This");
update t1 set c1="That" where c1="This";
affected rows: 1
info: Rows matched: 1  Changed: 1  Warnings: 0
select * from t1;
c1
That
drop table t1;
create table t1 (a int not null) engine=csv;
lock tables t1 read;
select * from t1;
ERROR HY000: File 'MYSQLD_DATADIR/test/t1.CSV' not found (OS errno 2 - No such file or directory)
unlock tables;
drop table t1;
CREATE TABLE t1 (e enum('foo','bar') NOT NULL) ENGINE = CSV;
INSERT INTO t1 VALUES();
INSERT INTO t1 VALUES(default);
INSERT IGNORE INTO t1 VALUES(0);
Warnings:
Warning	1265	Data truncated for column 'e' at row 1
INSERT IGNORE INTO t1 VALUES(3);
Warnings:
Warning	1265	Data truncated for column 'e' at row 1
INSERT IGNORE INTO t1 VALUES(-1);
Warnings:
Warning	1265	Data truncated for column 'e' at row 1
SELECT * FROM t1;
e



foo
foo
DROP TABLE t1;
#
# Test for the following cases
# 1) integers and strings enclosed in quotes
# 2) integers and strings not enclosed in quotes
# 3) \X  characters with quotes
# 4) \X  characters outside quotes
#
CREATE TABLE t1(c1 INT NOT NULL, c2 VARCHAR(50) NOT NULL) ENGINE=csv;
# remove the already existing .CSV file if any
# create the .CSV file that contains the hard-coded data used in
# testing
1,"integer sans quotes"
1,string sans quotes
1,quotes"in between" strings 
"1",Integer with quote and string with no quote
1,"escape sequence \n \" \\ \r \a within quotes"
1,escape sequence \n \" \\ \r \a without quotes
# select from the table in which the data has been filled in using
# the hard-coded .CSV file
SELECT * FROM t1;
c1	c2
 " \  \a within quotes
 " \  \a without quotes
1	Integer with quote and string with no quote
1	escape sequence 
1	escape sequence 
1	integer sans quotes
1	quotes"in between" strings 
1	string sans quotes
DROP TABLE t1;
# Test for the case when a field begins with a quote, but does not end in a
# quote.
# Note: This results in an error.
CREATE TABLE t1(c1 INT NOT NULL, c2 VARCHAR(50) NOT NULL) ENGINE=csv;
# remove the already existing .CSV file if any
# create the .CSV file that contains the hard-coded data used in
# testing
1,"string only at the beginning quotes
# select from the table in which the data has been filled in using
# the hard-coded .CSV file
SELECT * FROM t1;
ERROR HY000: Table 't1' is marked as crashed and should be repaired
DROP TABLE t1;
# Test for the case when a field ends with a quote, but does not begin in a
# quote.
# Note: This results in an error.
CREATE TABLE t1(c1 INT NOT NULL, c2 VARCHAR(50) NOT NULL) ENGINE=csv;
# remove the already existing .CSV file if any
# create the .CSV file that contains the hard-coded data used in
# testing
1,string with only ending quotes"
# select from the table in which the data has been filled in using
# the hard-coded .CSV file
SELECT * FROM t1;
ERROR HY000: Table 't1' is marked as crashed and should be repaired
DROP TABLE t1;
End of 5.1 tests

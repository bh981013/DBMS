select t.name, count(c.pid)
from Trainer t, CatchedPokemon c
where t.id = c.owner_id and t.hometown = 'Sangnok City'
group by t.id
order by count(c.pid);